#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "serveur.h"

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


#define TRUE 1
#define FALSE 0
#define LONGUEUR_TAMPON 4096
#define LIMIT_CHAR 30


#ifdef WIN32
#define perror(x) printf("%s : code d'erreur : %d\n", (x), WSAGetLastError())
#define close closesocket
#define socklen_t int
#endif

/* Variables cachees */

/* le socket d'ecoute */
int socketEcoute;
/* longueur de l'adresse */
socklen_t longeurAdr;
/* le socket de service */
int socketService;
/* le tampon de reception */
char tamponClient[LONGUEUR_TAMPON];
int debutTampon;
int finTampon;


/* Initialisation.
 * Creation du serveur.
 */
int initialisation() {
	return initialisationavecservice("13214");
}

/* Initialisation.
 * Creation du serveur en pr�cisant le service ou num�ro de port.
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int initialisationavecservice(char *service) {
	int n;
	const int on = 1;
	struct addrinfo	hints, *res, *ressave;

	#ifdef WIN32
	WSADATA	wsaData;
	if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR)
	{
		printf("WSAStartup() n'a pas fonctionne, erreur : %d\n", WSAGetLastError()) ;
		WSACleanup();
		exit(1);
	}
	memset(&hints, 0, sizeof(struct addrinfo));
    #else
	bzero(&hints, sizeof(struct addrinfo));
	#endif

	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(NULL, service, &hints, &res)) != 0)  {
     		printf("Initialisation, erreur de getaddrinfo : %s", gai_strerror(n));
     		return 0;
	}
	ressave = res;

	do {
		socketEcoute = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (socketEcoute < 0)
			continue;		/* error, try next one */

		setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
#ifdef BSD
		setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
#endif
		if (bind(socketEcoute, res->ai_addr, res->ai_addrlen) == 0)
			break;			/* success */

		close(socketEcoute);	/* bind error, close and try next one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {
     		perror("Initialisation, erreur de bind.");
     		return 0;
	}

	/* conserve la longueur de l'addresse */
	longeurAdr = res->ai_addrlen;

	freeaddrinfo(ressave);
	/* attends au max 4 clients */
	listen(socketEcoute, 4);
	printf("Creation du serveur reussie sur %s.\n", service);

	return 1;
}

/* Attends qu'un client se connecte.
 */
int attenteclient() {
	struct sockaddr *clientAddr;
	char machine[NI_MAXHOST];

	clientAddr = (struct sockaddr*) malloc(longeurAdr);
	socketService = accept(socketEcoute, clientAddr, &longeurAdr);
	if (socketService == -1) {
		perror("AttenteClient, erreur de accept.");
		return 0;
	}
	if(getnameinfo(clientAddr, longeurAdr, machine, NI_MAXHOST, NULL, 0, 0) == 0) {
		printf("Client sur la machine d'adresse %s connecte.\n", machine);
	} else {
		printf("Client anonyme connecte.\n");
	}
	free(clientAddr);
	/*
	 * Reinit buffer
	 */
	debutTampon = 0;
	finTampon = 0;

	return 1;
}


/* Recoit un message envoye par le serveur.
 */
char *reception() {
	char message[LONGUEUR_TAMPON];
	int index = 0;
	int fini = FALSE;
	int retour = 0;
	while(!fini) {
		/* on cherche dans le tampon courant */
		while((finTampon > debutTampon) &&
			(tamponClient[debutTampon]!='\n')) {
			message[index++] = tamponClient[debutTampon++];
		}
		/* on a trouve ? */
		if ((index > 0) && (tamponClient[debutTampon]=='\n')) {
			message[index++] = '\n';
			message[index] = '\0';
			debutTampon++;
			fini = TRUE;
#ifdef WIN32
			return _strdup(message);
#else
			return strdup(message);
#endif
		} else {
			/* il faut en lire plus */
			debutTampon = 0;
			retour = recv(socketService, tamponClient, LONGUEUR_TAMPON, 0);
			if (retour < 0) {
				perror("Reception, erreur de recv.");
				return NULL;
			} else if(retour == 0) {
				fprintf(stderr, "Reception, le client a ferme la connexion.\n");
				return NULL;
			} else {
				/*
				 * on a recu "retour" octets
				 */
				finTampon = retour;
			}
		}
	}
	return NULL;
}




/* Recoit des donnees envoyees par le client.
 */
int receptionbinaire(char *donnees, size_t tailleMax) {
	size_t dejaRecu = 0;
	int retour = 0;
	/* on commence par recopier tout ce qui reste dans le tampon
	 */
	while((finTampon > debutTampon) && (dejaRecu < tailleMax)) {
		donnees[dejaRecu] = tamponClient[debutTampon];
		dejaRecu++;
		debutTampon++;
	}
	/* si on n'est pas arrive au max
	 * on essaie de recevoir plus de donnees
	 */
	if(dejaRecu < tailleMax) {
		retour = recv(socketService, donnees + dejaRecu, tailleMax - dejaRecu, 0);
		if(retour < 0) {
			perror("ReceptionBinaire, erreur de recv.");
			return -1;
		} else if(retour == 0) {
			fprintf(stderr, "ReceptionBinaire, le client a ferme la connexion.\n");
			return 0;
		} else {
			/*
			 * on a recu "retour" octets en plus
			 */
			return dejaRecu + retour;
		}
	} else {
		return dejaRecu;
	}
}

/* Envoie des donn�es au client en pr�cisant leur taille.
 */
int emissionbinaire(char *donnees, size_t taille) {
	int retour = 0;
	retour = send(socketService, donnees, taille, 0);
	if(retour == -1) {
		perror("Emission, probleme lors du send.");
		return -1;
	} else {
		return retour;
	}
}

/* Ferme la connexion avec le client.
 */
void terminaisonclient() {
	close(socketService);
}

/* Arrete le serveur.
 */
void terminaison() {
	close(socketEcoute);
}

/*---------------------------------Our code-----------------------------------*/

//Vous trouverez sous cette partie tout le code qui a été écrit et/ou modifié
//afin de répondre aux besoins de notre projet.
//
//Remarque : les retours négatifs correspondent aux erreurs
//					un retour 0 correspond à une sortie depuis la fonction avec succès.

/*--------------------Fonction d'emission vers le client----------------------*/

//Cette fonction (reprise) permet d'envoyer les requêtes au client

int emission(char *message) {
	//Déclaration des variables
	int taille;

	if(strstr(message, "$*") == NULL) {		//On s'assure que la requête se termine bien par '$*'
		fprintf(stderr, "Emission, Le message n'est pas terminé par $*.\n");
		return 0;
	}
	taille = strlen(message);
	if (send(socketService, message, taille,0) == -1) {
		perror("Emission, problème lors du send.");
		return 0;
	}
	//printf("Emission de %d caractères.\n", taille+1);
	printf("J'envois au client la requête suivante : %s\n", message);
	return 1;
}


/*-------------------Fonction vérification de l'allocation--------------------*/

//Cette fonction permet de vérifier si un élément est alloué ou non en mémoire

int vide(void *connerie){		//Si le pointeur pointe vers une zone vide
	return (connerie == NULL);		//Retourne NULL
}


/*------------------------Etude du format du message--------------------------*/

//Fonction permettant de vérifier si le message envoyé par le client respecte
//bien le formalisme/syntaxe que nous définissons dans cette fonction.

int analyser(char *requete_recue){
	//Déclaration des variables
	int taille;
	int i=0;	//Mise à 0 de la variable compteur

	if (vide(requete_recue))		//Si la requete_recue est incorrecte
		return -1;

	taille = strlen(requete_recue);
//Recherche du '$' tout en ne dépassant pas la taille Max de la requete_recue
	while((requete_recue[i] !='$') && (i<taille))
		i++;

	if(i>=taille)
		return -2;		//Retour d'erreur si le message est trop grand

	if(requete_recue[i+1]=='*')
		return 0;		//On sort de la fonction quand on aura rencontré une '*'
	else
		return -1;		//Erreur syntaxe incorrecte
}


/*---------------Formatage du message avant envoi au client-------------------*/

//Cette fonction s'assure d'envoyer un message au client en ajoutant la chaine
// de fin de message '$*' afin qu'il soit traité correctement par le client.
int format_message(char *message){
	//Déclaration des variables
	char *temp = calloc(strlen(message)+2, sizeof(char));	//Ajoute 2 places pour accueillir '$*'

	if ((vide(message)) || (vide(temp)))
		return -1;
//Ajoute la chaine de caractère $*
	strcat(temp,"$*");

	if(emissionbinaire(temp, strlen(temp)) == -1)
		return -2;

	free(temp);
	return 0;
}

/*-----------------------Extraction des paramètres----------------------------*/

//Fonction d'extraction des sous chaines (utilisée pour extraire les paramètres)
// exemple : a = extraction(30,p,login,'/');

int extraction(int lg_chaine, char *source, char *destination, char condition){
//Déclaration des variables
	int i = 0; //variable compteur

	if (!lg_chaine){	//Si la longueur de ma chaine est incorrecte
		printf("[Extraction] Erreur longueur chaine\n");
		return -1;
	}

//On vérifie toujours si il n'ya pas d'allocation mémoire erronée
	if ((vide(source)) || (vide(destination)) || (vide(&condition))){
		printf("[Extraction] Erreur allocation\n");
		return -2;
	}

//J'extrais le paramètre courant tant que je n'ai pas de caractère condition et
//et que la chaine respecte la taille imposée
	while((source[i] != condition) && (i<lg_chaine)){
		if (source[i] == condition)
			break;		//Un petit break par i par là ne fait pas de mal :)
		else
			destination[i] = source[i];
		i++;
	}
	destination[i] = '\0';
	source++;		//On se place après le '/' à la fin de l'extraction
	return 0;
}


/*-----------------------------Authentification-------------------------------*/

//C'est cette fonction qui nous permettra de s'assurer que la personne qui
//tente de s'authentifier fait partie de notre whitelist.
int authentification(char *message, char* login_extrait, char* paswd_extrait){
//Déclaration des variables
	char *p = (char *) calloc(50,sizeof(char));		//Tableau qui va stocker temporairement le message
	char *login = (char *) calloc(30,sizeof(char));		//Contiendra le login
	char *paswd = (char *) calloc(30,sizeof(char));		//Contiendra le mot de passe
	int resultat;		//Entier qui servira de retour


	p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
	// p = /paramètre1/paramètre2$*
	p++;		//debut du 1er paramètre

	if (vide(p))
		return -1;

	//On extrait dans le p tous les caractères jusqu'au prochain '/' qui
	//correspondra au login
	resultat = extraction(30, p, login, '/');
	// p = la meme chose mais login = 1er paramètre
	if (resultat ==0){
		p = strstr(p,"/");
		p++;
		//On extrait dans le p tous les caractères jusqu'au prochain '/' qui
		//correspondra au mot de passe
		resultat = extraction(30, p, paswd, '$');

		if (resultat < 0){
			printf("[Authentification] L'extraction du mot de passe a échoué !\n");
			return -2;
		}
	}
	else{
		printf("[Authentification] L'extraction du login a échoué !\n");
		return -3;
	}
		//On copie le login et le paswd dans des variables de sortie pour les
		//utiliser en dehors de la fonction Authentification
		strcpy(login_extrait, login);
		strcpy(paswd_extrait, paswd);

	return 0;
}


/*-----------------------Ouverture fichier identifiants-----------------------*/

//Fonction complémentaire à la fonction Authentification qui va comparer
//les creditentiels saisies par le client aux données d'authentification
//présents dans le fichier .

int ouverture_id(char* login_recu){
//Déclaration des variables
	int resultat;
	char c;
	FILE* fichier;		//On va ouvrir un fichier
	int i = 0;
	int stop = 0;

	printf("Login reçu : %s\n", login_recu);
	//Ouverture du fichier identifiants.txt du repertoire courant en lecture seule
	fichier = fopen("identifiants.txt", "r");

	if (vide(fichier)){
		printf("[Ouverture_id] Ouverture du fichier identifiants impossible !\n");
		return -1;
	}
	else
		printf("Ouverture fichier identifiants réussie\n");

		//Variable qui va permettre de stocker ce qui est lu dans le fichier
	char *carac_lus;
	do{
		carac_lus = (char *) calloc(30,sizeof(30));
		do{
					c = fgetc(fichier);

					if (!feof(fichier)){ //Si la fin du fichier est atteinte, je coupe la boucle
						if (c == '\n'){
							carac_lus[i] = '\0';
							stop = 1;
						}
						else{
							carac_lus[i] = c;
							i++;
						}
					}
				}while(!feof(fichier)&&(!stop));
				stop = 0;
				i=0;
			//Tant qu'on a pas atteint la fin du fichier et que les login_recu=carac_lus
			}while(!feof(fichier) && (strcmp(login_recu,carac_lus)));
	printf("Login reçu\":%s\"\nLogin lu:\"%s\"\n",login_recu,carac_lus);
// On ferme le fichier qui a été ouvert
	if(fclose(fichier) == -1){
		printf("[Ouverture_id] Erreur, le fichier n'a pas pu se fermer correctement !\n");
		return -1;
	}
	else
		printf("Le fichier a été fermé avec succès\n");

	if (!strcmp(login_recu,carac_lus)){
		printf("Les chaines sont identiques\n");
		free(carac_lus);
		return 0;
	}

	free(carac_lus);		//On libère le carac_lus
	return -2;

	//return 0;
}


/*-------------------------Ouverture du fichier paswd-------------------------*/

//Cette fonction est la suite de la fonction ouverture_id, qui permettra de
// vérifier si le mot de passe saisie par le client est correct.

int lecture_paswd(char *login, char *paswd){
	//Déclaration des variables
	FILE* fichier;
	char *chemin = (char*) calloc(100,sizeof(char));
	char *carac_lus = (char*) calloc(30,sizeof(char));
	char c;
	int i = 0;
	int stop = 0;

//On va concaténer au fur et à mesure le chemin conduisant vers le fichier paswd
	strcat(chemin,"./users/");		//on se place dans le dossier /users
	strcat(chemin,login);		//Le dossier du client correspond à son login
	strcat(chemin,"/paswd.txt");		//Le fichier contenant le mdp s'appellerons toujours ainsi
//Ce qui nous donne chemin = ./users/login/paswd.txt

	fichier = fopen(chemin, "r");		//Ouverture du fichier paswd en lecture seule
	if (vide(fichier)){
		printf("[lecture_paswd] Ouverture du fichier password impossible !\n");
		return -1;
	}

	do{
					c = fgetc(fichier);
					if (!feof(fichier)){ //si la fin du fichier est atteinte, je coupe la boucle
						if (c == '\n'){
							carac_lus[i] = '\0';
							stop = 1;
						}
						else{
							carac_lus[i] = c;
							i++;
						}
					}
					else{
						carac_lus[i] = '\0';
						return -2;
				}
	}while(!feof(fichier)&&(!stop));

	printf("lu: %s, recu:%s\n", carac_lus, paswd);
// On ferme le fichier qui a été ouvert
	if(fclose(fichier) == -1){
		printf("[lecture_paswd] Erreur, le fichier n'a pas pu se fermer correctement !\n");
		return -1;
	}
	else
		printf("Le fichier a été fermé avec succès\n");

	if (!strcmp(paswd,carac_lus)){
		printf("Les chaines sont identiques\n");
		free(carac_lus);
		return 0;
	}
	free(carac_lus);
	return -2;

	// return 0;
	}
/*----------------------------Liste des messages------------------------------*/

//Fonction permettant d'envoyer le contenu de la boite de récéption au client.

int content(char chemin[256]){
  //Declaration des variables utilisées dans cette fonction
    DIR* dossier;
    FILE* fichier_lu;
    struct dirent* fichier = NULL;
  	char carac_lus;
  	char *c;
    char chemin_pour_lire[256];
  	int i = 0;
  	int stop = 0;
    char requete[20];
    char variable_envoye[20] = "/n";

  //Ouverture du dossier contenant les messages de l'utilisateur
    dossier = opendir(chemin);
    if(dossier == NULL){
      printf ("Erreur, le dossier n'a pas pu être ouvert !\n");
      return -1;
    }else{
      printf("Le dossier a été ouvert avec succès\n");
    }

  //Exploitation des fichiers
  while((fichier = readdir(dossier)) != NULL){

    // les dossiers parents et actuels (. et ..) ne sont pas lus
    if (strcmp(fichier->d_name, ".") != 0 && strcmp(fichier->d_name, "paswd.txt") != 0 && strcmp(fichier->d_name, "..") != 0){

      //Préparation du chemin utilisé dans les manipulations du fichier
      strcpy (chemin_pour_lire,chemin);
			//On concatène le chemin
			strcat(chemin_pour_lire, "/");
      strcat(chemin_pour_lire, fichier->d_name);


      //Envoi du numéro du message
      sprintf(requete, "Content/%s$*\n", fichier->d_name);
      //printf("%s\n", requete);
			emission(requete);
      //Ouverture du fichier_lu
      fichier_lu = fopen (chemin_pour_lire, "r");
      //Test si le fichier est bien ouvert pour pouvoir travailler
      if (fichier_lu != NULL){
        i=0;
         // On peut lire et écrire dans le fichier
           while ((fgets(variable_envoye, 20, fichier_lu) != NULL) && (i != 2)){ // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
              // supression de /n afin de mettre en forme selon notre RFC
              variable_envoye[strlen(variable_envoye)-1]=0;
              sprintf(requete, "Content/%s$*\n", variable_envoye);
              //printf("%s\n", requete);
							emission(requete);
              i = i + 1;
           }
           printf("\n");
           fclose (fichier_lu);
       }else{
         // On affiche un message d'erreur si l'on veut
         printf("[content] Impossible d'ouvrir le message !");
       }
    }


 }
 emission("Reply/606$*\n");

// fermeture du dossier de l'utilisateur
  if (closedir(dossier) == -1){
    printf("[content] Erreur, le dossier n'a pas pu se fermer correctement !\n\n");
    return -1;
  }else{
    printf("Le dossier a été fermé avec succès\n\n");
  }

  return 0;
}

/*---------------------------Nombre de messages-------------------------------*/

//Fonction qui compte le nombre de fichier dans un dossier,
//Utilisée ici pour compter le nombre de message dans une boite mail

int nombre_msg_boite_mail (char *chemin){
//Déclaration des variables utilisées dans cette fonction
  DIR* dossier;
  int nbr = 0;
  struct dirent* fichier = NULL;

//Ouverture du dossier contenant les messages de l'utilisateur
  dossier = opendir(chemin);
  if(dossier == NULL){
    printf ("[nombre_msg_boite_mail] Erreur, le dossier n'a pas pu être ouvert !\n");
    return -1;
  }else{
    printf("Le dossier a été ouvert avec succès\n");
  }

//Comptage des fichier, ici les messages, de l'utilisateur
  while ((fichier = readdir(dossier)) != NULL)
  {
    if (strcmp(fichier->d_name, ".") != 0 && /* Si le fichier lu n'est pas . */
      strcmp(fichier->d_name, "..") != 0) /*  Et n'est pas .. non plus */
      nbr++; /* Alors on incrémente le compteur */
  }

// fermeture du dossier de l'utilisateur
  if (closedir(dossier) == -1){
    printf("[nombre_msg_boite_mail] Erreur, le dossier n'a pas pu se fermer correctement !\n");
    return -1;

  }else{
    printf("Le dossier a été fermé avec succès\n");
  }
	nbr = nbr - 1; /*On enlève le fichier passwd*/
  return nbr;

}


/*------------------------Suppression de messages-----------------------------*/

//Fonctions qui permet de supprimer un message que l'utilisateur aura choisi
//de supprimer.
//Remarque : numero_message doit representer le code ascii du mail visé

int supprimer_message (char chemin[256], char numero_message[3]){

	//Declaration des variables utilisées dans cette fonction
  //variables qui representent ici la boite mail de l'utilisateur
  DIR* dossier;
  //fichier represente ici, l'adresse mail de l'utilisateur
  struct dirent* fichier = NULL;
  char test = 0;
	char chemin_supprimer[256];
	strcpy (chemin_supprimer, chemin);


  // opendir ouvre le dossier indiqué par chemin
  dossier = opendir(chemin);
  if(dossier == NULL){
    printf ("[supprimer_message] Erreur, le dossier n'a pas pu être ouvert !\n\r");
    return -1;
  }else{
    printf("Le dossier a été ouvert avec succès\n\r");
  }

  //Lecture du nom du fichier
  // tant qu'on ne trouve pas le numero mail correspondant
  while((fichier = readdir(dossier)) != NULL){
    /*Dans notre cas si on trouve le mail correspondant au numero indiqué
      dans numero_message alors on suprime le mail et on incremente le test.
      cela signifie qu'il existe bien un mail dans la boite_mail de
      l'utilisateur.*/
    if (strcmp(fichier->d_name, numero_message) == 0){
			printf("Le fichier à supprimer est : %s\n", fichier->d_name);
			strcat (chemin_supprimer, "/");
			strcat (chemin_supprimer, fichier->d_name);
			if(remove (chemin_supprimer) == 0){
				printf("Il a été supprimer correctement.\n");
			}else{
				printf("[supprimer_message] Erreur de suppression !\n");
				return 1;
			}
      test = test + 1;
    }
  }

  if (closedir(dossier) == -1){
    printf("[supprimer_message] Erreur, le dossier n'a pas pu se fermer correctement !\n");
    return -1;
  }else{
    printf("Le dossier a été fermé avec succès\n");
  }

	// On test la variable test, si elle est égale a 0 c'est qu'aucun
	// mail n'a pas ete retrouve correspond au numero
	if(test == 0)
	{
		printf("Aucun message ne correspond à votre choix\n");
		return 1;
	}
	return 0;
}

/*---------------------------Lecture de messages------------------------------*/

//Cette fonction permet d'envoyer au client le contenu du mail

int mail(char chemin[256], char numero_message[3]){

//Declaration des variables utilisées dans cette fonction
  //Variable qui represente ici la boite mail de l'utilisateur
  DIR* dossier;
	//Variable qui represente ici le mail de l'utilisateur
	FILE* fichier_lu;
  //Fichier represente ici, le mail de l'utilisateur
  struct dirent* fichier = NULL;
  char test = 0;
  char variable_envoye[256];
  char requete[5000] = "\0";
  char requete_message[5000] = "\0";
  char chemin_pour_lire[256];


  //opendir ouvre le dossier indiqué par chemin
  dossier = opendir(chemin);
  if(dossier == NULL){
    printf ("[mail] Erreur, le dossier n'a pas pu être ouvert !\n\r");
    return -1;
  }else{
    printf("Le dossier a été ouvert avec succès\n\r");
  }

  //Lecture du nom du fichier
  // tant qu'on trouve pas le numero mail correspondant
  while((fichier = readdir(dossier)) != 0){

    if (strcmp(fichier->d_name, numero_message) == 0){
      strcpy (chemin_pour_lire,chemin);
       strcat(chemin_pour_lire, "/");
       strcat(chemin_pour_lire, fichier->d_name);
   	//printf("%s\n", chemin_pour_lire );
      fichier_lu = fopen (chemin_pour_lire, "r");

      //test si le fichier est bien ouvert pour pouvoir travailler
      if (fichier_lu != NULL){
						emission("Reply/101$*\n");
          //printf("le dossier a ouvrir est %s\n\n", fichier->d_name);

					// On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL) ou test ==2
            while ((test != 2)){
                    fgets(variable_envoye, 256, fichier_lu) != NULL;
                    // supression de /n afin de mettre en forme selon notre RFC
                    variable_envoye[strlen(variable_envoye)-1]= '\0';
                    //strcat(requete, variable_envoye);
                    sprintf(requete, "Mail/%s$*\n", variable_envoye);
										//printf("Requête envoyée : %s\n", requete);
                    emission(requete);

                    test = test +1;
             }

            test = 0;
             sprintf (requete, "%s", "\0");
             while ((fgets(requete_message, 5000, fichier_lu) != NULL)){

                     // supression de /n afin de mettre en forme selon notre RFC
                     //variable_envoye[strlen(variable_envoye)-1]= '#';

                  sprintf(requete, "Mail/%s$*\n", requete_message);

                  //printf("%s\n", requete);
                  test++;

              }
							 //requete_message[strlen(requete_message)-1] = 0 ;
               //sprintf(requete, "Mail/%s$*\n", requete_message);
							 //printf("Requête envoyée : %s\n", requete);
               emission(requete);
               //printf("\n");

            fclose(fichier_lu);
        }else{
          printf("[mail] Erreur ouverture message !\n");

        }
    }
  }
  // On test la variable test, si elle est égale a 0 c'est qu'aucun
  // mail n'a pas ete retrouve correspond au numero
  if(test == 0)
  {
    printf("[mail] Aucun message ne correspond à votre choix !\n");
		emission("Reply/404$*\n");
  }

  if (closedir(dossier) == -1){
    printf("[mail] Erreur, le dossier n'a pas pu se fermer correctement !\n");
    return -1;
  }else{
    printf("Le dossier a été fermé avec succès\n");
  }
	return 0;
}

/*---------------------------Stockage de messages-----------------------------*/

//Fonction permettant de créer un fichier contenant l'expéditeur, l'objet et
//le contenu du message. Il sera stocker dynamiquement dans le dossier du destinataire.

int stockage_message(char *message, char *login){
	//Déclaration des variables
	FILE* fichier;
	DIR* dossier;
	struct dirent* nom_fichier = NULL;
	char *p = (char *) calloc(4000,sizeof(char));
	char *chemin = (char*) calloc(100,sizeof(char));
	char *destinataire = (char *) calloc(30,sizeof(char));
	char *objet = (char *) calloc(50,sizeof(char));
	char *corps = (char *) calloc(4000,sizeof(char));
	int resultat;
	int num_message;
//variables test
	char *cheminTest = (char *) calloc(100,sizeof(char));
	FILE *nouveauMessage;
	int trouve = 0;

	p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
	p++;		//debut du 1er paramètre

	if (vide(p))
		return -1;


	resultat = extraction(30, p, destinataire, '$');	//on extrait le destinataire
	if (resultat < 0){
		printf("[stockage_message] L'extraction du destinataire a echoué !\n");
		return -2;
	}
	bzero(p, 4000); /*Remet à zero le tableau p*/

	//On concatène afin de créer le lien vers le dossier du destinataire
	strcat(chemin,"./users/");
	strcat(chemin,destinataire);

	printf("Destinataire : %s\n", destinataire);

	dossier = opendir(chemin);		//on tente d'ouvrir le dossier du destinataire
  if(dossier == NULL){
    printf ("[stockage_message] Erreur, le dossier n'a pas pu être ouvert !\n");
		return -1;
  }else{
		printf("Le dossier a été ouvert avec succès\n");
		emission("Reply/101$*\n");	//On envoi une réponse au client pour qu'il puisse continuer
	}

	receptionbinaire(message,50);		//On attend la requête

	p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
	p++;		//debut du 1er paramètre

	if (vide(p))
		return -1;

	resultat = extraction(50, p, objet, '$');
	if (resultat < 0){
		printf("[stockage_message] L'extraction de l'objet a eéhoué !\n");
		return -2;
	}
	bzero(p, 4000); //Remet à zero le tableau p

	printf("Objet : %s\n", objet);

	receptionbinaire(message,4000);

	p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
	p++;		//debut du 1er paramètre

	if (vide(p))
		return -1;

	resultat = extraction(4000, p, corps, '$');
	if (resultat < 0){
		printf("[stockage_message] L'extraction du corps a échoué !\n");
		return -2;
	}
	bzero(p, 4000); /*Remet à zero le tableau p*/

	printf("Corps : %s\n", corps);


	//Ouverture du fichier

	num_message = 1;	//On initialise le num_message à 1 pour commencer à 1

	//Cette boucle do sert à s'assurer que le nouveau fichier mail sera enregistré
	//sans ecraser un fichier existant en incrémentant sa valeur.

	int fd;

	do{
		strcat(cheminTest,chemin);
		sprintf(cheminTest, "%s/%d", cheminTest, num_message); //On concatène
		fd = open(cheminTest,O_RDONLY);

		if (fd != -1){	//Si l'ouverture se passe correctement
			close(fd);
			num_message++; //On l'incrémente pour ne pas créer de fichier qui existe déjà
		}
		else
			trouve = 1;
		memset(cheminTest,'\0',100);
	}while (!trouve);

	free(cheminTest);
	sprintf(chemin, "%s/%d",chemin, num_message); // C'est le bon chemin
	printf("Le mail sera stocké dans : %s\n",chemin);

	fichier = fopen(chemin, "w");	//On ouvre le fichier en écriture seule

	if (vide(fichier)){
		printf("[stockage_message] Ouverture du fichier mail impossible !\n");
		return -1;
	}

	fprintf(fichier, "%s\n%s\n%s\n", login, objet, corps);
// On ferme le fichier qui a été ouvert
	if(fclose(fichier) == -1){
		printf("[stockage_message] Erreur, le fichier n'a pas pu se fermer correctement !\n");
		return -1;
	}
	else
		printf("Le fichier a été fermé avec succès\n");

	if (closedir(dossier) == -1){
		printf("[stockage_message] Erreur, le dossier n'a pas pu se fermer correctement !\n");
		return -1;
	}else{
		printf("Le dossier a été fermé avec succès\n");
	}
	return 0;
}
