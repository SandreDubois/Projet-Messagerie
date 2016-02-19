
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#define RED      "\033[1;31m"
#define GREEN    "\033[1;32m"
#define YELLOW   "\033[1;33m"
#define BLUE     "\033[1;34m"
#define PURPLE   "\033[1;35m"
#define CYAN     "\033[1;36m"
#define GREY     "\033[1;37m"
#define DEFAULT_COLOR "\033[0;m"

#include "client.h"

#define TRUE 1
#define FALSE 0

#define LONGUEUR_TAMPON 4096

/* Variables cachees */

/* le socket client */
int socketClient;
/* le tampon de reception */
char tamponClient[LONGUEUR_TAMPON];
int debutTampon;
int finTampon;

/* Initialisation.
 * Connexion au serveur sur la machine donnee.
 * Utilisez localhost pour un fonctionnement local.
 */
int Initialisation(char *machine) {
	return InitialisationAvecService(machine, "13214");
}

/* Initialisation.
 * Connexion au serveur sur la machine donnee et au service donne.
 * Utilisez localhost pour un fonctionnement local.
 */
int InitialisationAvecService(char *machine, char *service) {
	int n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(machine, service, &hints, &res)) != 0)  {
     		fprintf(stderr, "Initialisation, erreur de getaddrinfo : %s", gai_strerror(n));
     		return 0;
	}
	ressave = res;

	do {
		socketClient = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (socketClient < 0)
			continue;	/* ignore this one */

		if (connect(socketClient, res->ai_addr, res->ai_addrlen) == 0)
			break;		/* success */

		close(socketClient);	/* ignore this one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {
     		perror("Initialisation, erreur de connect.");
     		return 0;
	}

	freeaddrinfo(ressave);
	printf("Connexion avec le serveur reussie.\n");

	return 1;
}

/* Recoit un message envoye par le serveur.
 */
char *Reception() {
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
			return strdup(message);
		} else {
			/* il faut en lire plus */
			debutTampon = 0;
			retour = recv(socketClient, tamponClient, LONGUEUR_TAMPON, 0);
			if (retour < 0) {
				perror("Reception, erreur de recv.");
				return NULL;
			} else if(retour == 0) {
				fprintf(stderr, "Reception, le serveur a ferme la connexion.\n");
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

/* Envoie un message au serveur.
 * Attention, le message doit etre termine par \n
 */
int Emission(char *message) {
	if(strstr(message, "$*\n") == NULL) {
		fprintf(stderr, "Emission, Le message n'est pas termine par $*.\n");
	}
	int taille = strlen(message);
	if (send(socketClient, message, taille,0) == -1) {
        perror("Emission, probleme lors du send.");
        return 0;
	}
	//printf("Emission de %d caracteres.\n", taille+1);
	return 1;
}

/* Recoit des donnees envoyees par le serveur.
 */
int ReceptionBinaire(char *donnees, size_t tailleMax) {
	int dejaRecu = 0;
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
		retour = recv(socketClient, donnees + dejaRecu, tailleMax - dejaRecu, 0);
		if(retour < 0) {
			perror("ReceptionBinaire, erreur de recv.");
			return -1;
		} else if(retour == 0) {
			fprintf(stderr, "ReceptionBinaire, le serveur a ferme la connexion.\n");
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

/* Envoie des donn�es au serveur en pr�cisant leur taille.
 */
int EmissionBinaire(char *donnees, size_t taille) {
	int retour = 0;
	retour = send(socketClient, donnees, taille, 0);
	if(retour == -1) {
		perror("Emission, probleme lors du send.");
		return -1;
	} else {
		return retour;
	}
}


/* Ferme la connexion.
 */
void Terminaison() {
	close(socketClient);
}

/*___________________________________________DEBUT______________________________________________*/

/*Fonction pour vider le buffer*/
void FreeBuffer(){
    int c = 0;
    while (c != '\n' && c != EOF)
		{
        c = getchar();
    }
}

/*Fonction pour revenir au menu principal avec "Entrée" et rester dans la fonction en attendant*/
int RetourMenuPrecedent(){
	char c;
	FreeBuffer();
	scanf("%c", &c);
	if (c == '\n'){
		return 0;
	}
	return 1;
}

int RetourMenuPrecedent_2(){
	char c;
	scanf("%c", &c);
	if (c == '\n'){
		return 0;
	}
	return 1;
}

/*-----------------------Extraction des paramètres----------------------------*/


//Fonction d'extraction des sous chaines (utilisée pour extraire les paramètres)
// exemple : a = extraction(30,p,login,'/');

int extraction(int lg_chaine, char *source, char *destination, char condition){

	int i = 0;

	if (!lg_chaine){
		printf("[Extraction] Erreur longueur chaine\n");
		return -1;
	} 		//à commenter

//J'extrais le paramètre courant tant que je n'ai pas de caractère condition et
//et que la chaine respecte la taille imposée
	while((source[i] != condition) && (i<lg_chaine)){
		if (source[i] == condition)
			break;
		else
			destination[i] = source[i];
		i++;
	}
	destination[i] = '\0';
	//printf("extraction: %s",destination);
	source++;		//on se place après le '/' à la fin de l'extraction
	return 0;
}

/*______________________________Affichage menu connexion_______________________________________*/
void Menu_Authentification(){
	printf("***************************** %sMENU%s ****************************\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sTMP Messagerie%s                       *\n", BLUE, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                    %sMenu d'Authentification%s                  *\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");	/*Affichage Menu_Authentification*/
  printf("*************** %s1 - Pour vous authentifier %s********************\n", YELLOW, DEFAULT_COLOR);
  printf("*************** %s2 - Pour vous déconnecter  %s********************\n", YELLOW, DEFAULT_COLOR);
	printf("***************************************************************\n");
	printf("*                                                             *\n");
	printf("* %sProjet de Frederic FERRERA, Saidaran SARMA et Sandre DUBOIS%s *\n", BLUE, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FFSSSD ************************** Version 1.0 ***\n");
	printf("\n");
}

/*____________________________________Connexion au serveur______________________________________*/
int Authentification(){
	/*Déclaration des variables*/
	char *message = calloc(50, sizeof (char));
	int rep;	/*Permet de stocker le type de réponse du serveur*/
	char adresse_client[30];	/*Permet de stocker l'adresse mail du client dans un tableau de 30 caractères'*/
	char mdp_client[30];	/*Permet de stocker le mot de passe du client dans un tableau de 20 caractères'*/
	char requete[80];	/*Permet de stocker la requête complete concatené dans un tableau de 80 caractères'*/

	/*En-tête menu authentification*/
	printf("***************************** %sMENU%s ****************************\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sTMP Messagerie%s                       *\n", BLUE, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                    %sMenu d'Authentification%s                  *\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FFSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Récupération de l'adresse mail*/
	printf("Veuillez saisir votre adresse mail :\n");
	FreeBuffer(); /*On vide le buffer, pour eviter toutes erreurs ultérieures*/
	fgets(adresse_client, 30, stdin);	/*On récupère la saisie du clavier qui est l'adresse mail
	 																	dans la variable "adresse_client"*/
	adresse_client[strlen(adresse_client)-1] = '\0';	/*On retire le "\n" à la requête car fgets met
																										automatiquement un "\n" à la fin*/

	printf("\n");	/*Retour à la ligne pour un affichage plus clair*/

	/*Récupération du mot de passe*/
	printf("Veuillez saisir votre mot de passe :\n");
	//FreeBuffer(); /*On vide le buffer, pour eviter toutes erreurs ultérieures*/
	fgets(mdp_client, 20, stdin);	/*On récupère la saisie du clavier qui est le mot de passe
	 															dans la variable "mdp_client"*/
	mdp_client[strlen(mdp_client)-1] = '\0'; /*Suppression du "\n" à la fin*/

	/*Concatenation des différents éléments*/
	sprintf(requete, "Authentification/%s/%s$*\n", adresse_client, mdp_client);	/*On concatene les identifiants
																																							et mot de passe avec la requête
																																							pour avoir la syntaxe
																																							"Authentification/identifiant/motdepasse$*"*/

	/*Test de l'émission de la requete*/
	if(Emission(requete)!=1) { /*On test si l'envoie de la requete s'est faite correctement sinon erreur lors de l'émission*/
		printf("Erreur lors de l'émission de l'adresse mail du mot de passe.\n"); /*Affichage message d'erreur*/
		return 1;
	}

	/*Réception de la réponse du serveur*/
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message,"Reply/%d$*",&rep); /*On extrait le paramètre de la reponse reçu,
																		qui correspond à l'état de l'authentification*/

	/*Exploitation de la réponse du serveur*/
	if(rep == 101){	/*Si on reçoit un "Reply/101$*", alors authentification correcte*/
		printf("\n");
		printf("Authentification Réussie.\n");	/*Affichage message d'erreur*/
	} else {
		if (rep == 202){	/*Si on reçoit un "Reply/202$*", alors authentification échec*/
			printf("\n");
			printf("Erreur lors de l'authentification.\n");
			printf("Votre adresse mail et le mot de passe ne correspondent pas.\n");
			printf("Retour au menu precedent, veuillez patienter.\n");
			system("sleep 5");
			free(message);
			// bzero(adresse_client, 30);
			// bzero(mdp_client, 30);
			// bzero(requete, 80);
			return 1;
		} else { /*Sinon erreur inconnue*/
			printf("\n");
			printf("Erreur inconnue.\n");	/*Affichage message d'erreur*/
			printf("Retour au menu precedent, veuillez patienter.\n");
			system("sleep 5");
			free(message);
			// bzero(adresse_client, 30);
			// bzero(mdp_client, 30);
			// bzero(requete, 80);
			return 1;
		}
	}
	free(message);
	// bzero(adresse_client, 30);
	// bzero(mdp_client, 30);
	// bzero(requete, 80);
	return 0;	/*La fonction retourne 0 si elle s'execute correctement*/
}


/*________________________________Affichage menu principal______________________________________*/
void Menu_Principal(){
	printf("***************************** %sMENU%s ****************************\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sTMP Messagerie%s                       *\n", BLUE, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sMenu Principal%s                       *\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");	/*Affichage Menu_Principal*/
	printf("************ %s1 - Consulter votre boite mail        %s************\n", YELLOW, DEFAULT_COLOR);
  printf("************ %s2 - Pour lire un mail                 %s************\n", YELLOW, DEFAULT_COLOR);
	printf("************ %s3 - Pour supprimer un mail            %s************\n", YELLOW, DEFAULT_COLOR);
	printf("************ %s4 - Pour envoyer un mail              %s************\n", YELLOW, DEFAULT_COLOR);
	printf("************ %s5 - Pour obtenir le nombre de message %s************\n", YELLOW, DEFAULT_COLOR);
	printf("************ %s6 - Pour vous déconnecter             %s************\n", YELLOW, DEFAULT_COLOR);
	printf("***************************************************************\n");
	printf("*** Copyright FFSSSD ************************** Version 1.0 ***\n");
	printf("\n");
}

/*_________________Récupération du choix de l'utilisateur pour les menus_______________________*/
int Choix(){
	int choix;
	if (scanf("%1d", &choix) != 1){
		//printf("erreur ce n'est pas un entier");
		return -1;	/*Retourne 8 si ce n'est pas un entier*/
	}else{
		return choix; /*Retourne choix si scanf OK*/
	}
}

/*_________________________Consultation de la boite de reception_______________________________*/
int Consult(int nombre_message_boite_mail){
	/*Déclaration des variables*/
	char *message = NULL;
	char *mail_numero = (char *) calloc(3, sizeof (char));; /*Permet de stocker le numéro du mail envoyé par le serveur*/
	char *mail_expediteur = (char *) calloc(30, sizeof (char));; /*Permet de stocker l'expéditeur du mail envoyé par le serveur*/
	char *mail_objet = (char *) calloc(30, sizeof (char));; /*Permet de stocker l'objet du  mail envoyé par le serveur*/
	char *p = (char *) calloc(4000,sizeof(char));
	int rep;	/*Permet de stocker le type de réponse du serveur*/
	int resultat; /*Permet de stocker le resultat de l'extraction*/
	int i = 0; /*Compteur pour le while*/

	/*En-tête menu Suppression message*/
	printf("***************************** %sMENU%s ****************************\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sTMP Messagerie%s                       *\n", BLUE, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*              %sConsultation de la boite de reception          %s*\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FFSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Envoie de la requête "Consult$*" pour demander la boite de reception*/
	/*Test de l'émission de l'envoie de la requete*/
	if(Emission("Consult/$*\n")!=1) {
		printf("Erreur d'emission lors de l'envoie de Consult.\n");	/*Affichage message d'erreur*/
		return 1;
	}

	do {
		message = Reception();
		p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
		p++;		//debut du 1er paramètre
		//resultat est le retour de l'extraction
		resultat = extraction(3, p, mail_numero, '$');
		if (resultat < 0){
			printf("L'extraction du destinataire a echoue\n");
			free(mail_numero);
			return -2;
		}

		message = Reception();
		p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
		p++;		//debut du 1er paramètre
		//resultat est le retour de l'extraction
		resultat = extraction(30, p, mail_expediteur, '$');
		if (resultat < 0){
			printf("L'extraction du destinataire a echoue\n");
		free(mail_expediteur);
			return -2;
		}

		message = Reception();
		p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
		p++;		//debut du 1er paramètre
		//resultat est le retour de l'extraction
		resultat = extraction(30, p, mail_objet, '$');
		if (resultat < 0){
			printf("L'extraction du destinataire a echoue\n");
			free(mail_objet);
			return -2;
		}

		/*Affichage de l'en-tête du mail*/
		printf("\n");
		printf("----------------------------%sMail n°%s%s--------------------------\n", GREY, mail_numero, DEFAULT_COLOR);
		printf("%sExpéditeur :%s %s\n", GREY, DEFAULT_COLOR, mail_expediteur);
		printf("%sObjet :%s %s\n", GREY, DEFAULT_COLOR, mail_objet);
		i++;
		free(message);
		printf("Message : %s\n", message);
	} while ( i != nombre_message_boite_mail );

	printf("\n");
	printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
	return 0;
}

/*__________________________________Lecture d'un messages______________________________________*/
int Read(){
	/*Déclaration des variables*/
	char *message = NULL;
	char *mail_expediteur = (char *) calloc(30, sizeof(char)); /*Permet de stocker l'expéditeur du mail envoyé par le serveur*/
	char *mail_objet = (char *) calloc(256, sizeof(char)); /*Permet de stocker l'objet du  mail envoyé par le serveur*/
	char *mail_contenu = (char *) calloc(5000, sizeof(char)); /*Permet de stocker le contenu mail envoyé par le serveur*/
	char *p = (char *) calloc(4000,sizeof(char));
	int num_message; /*Permet de stocker le numéro du message*/
	char requete[20];	/*Permet de stocker la requête complete concatené dans un tableau de 80 caractères'*/
	char remplacement = '\n'; /*Permet de stocker un caractère pour remplacer les "#" contenu dans le contenu du mail*/
	int i = 0; /*Variable utilisée pour la boucle permettant de parcourir le contenu du mail*/
	int resultat; /*Permet de stocker le resultat de l'extraction*/
	int rep = 0;	/*Permet de stocker le type de réponse du serveur*/

	/*En-tête menu Suppression message*/
	printf("***************************** %sMENU%s ****************************\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sTMP Messagerie%s                       *\n", BLUE, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sLecture Message%s                      *\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FFSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Récupération du nombre de messages à effacer*/
	printf("Quel message voulez-vous lire :\n");
	FreeBuffer();	/*On vide le buffer, pour eviter toutes erreurs ultérieure*/
	if (scanf("%d", &num_message) != 1){	/*Récupération du numéro de message et erreur
																				si c'est pas un entier*/
		printf("\n");
		printf("Veuillez saisir un entier.\n");
		printf("\n");
		printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
		return 1;	/*On sort de la fonction si c'est pas un entier*/
	}

	/*Concatenation des différents éléments*/
	sprintf(requete, "Read/%d$*\n", num_message);	/*On concatene le nombre de message avec la requête
																									pour avoir la syntaxe "Delete/nombredemessage$*"*/


	/*Test de l'émission de l'envoie de la requete*/
	if(Emission(requete)!=1) {
		printf("Erreur lors de l'émission du fichier à lire.\n");
		return 1;
	}

	/*Réception de la réponse du serveur*/
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message,"Reply/%d$*",&rep);	/*On extrait le paramètre de la reponse reçu,
																			qui correspond à l'état de la lecture du mail*/
	printf("Reply : %s\n", message);
	if(rep == 101){
		message = Reception();
		p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
		p++;		//debut du 1er paramètre
		//resultat est le retour de l'extraction
		resultat = extraction(30, p, mail_expediteur, '$');
		if (resultat < 0){
			printf("L'extraction de l'expediteur a echoue\n");
			free(mail_expediteur);
			return -2;
		}
		// printf("Mail exp : %s\n", mail_expediteur);
		// printf("Message : %s\n", message);

		message = Reception();
		p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
		p++;		//debut du 1er paramètre
		//resultat est le retour de l'extraction
		resultat = extraction(256, p, mail_objet, '$');
		if (resultat < 0){
			printf("L'extraction de l'objet a echoue\n");
			free(mail_objet);
			return -2;
		}
		// printf("Mail obj : %s\n", mail_objet);
		// printf("Message : %s\n", message);

		message = Reception();
		p = strstr(message,"/");	//lecture de la sous chaine jusqu'au premier '/'
		p++;		//debut du 1er paramètre
		//resultat est le retour de l'extraction
		resultat = extraction(5000, p, mail_contenu, '$');
		if (resultat < 0){
			printf("L'extraction du contenu a echoue\n");
			free(mail_contenu);
			return -2;
		}
		//printf("Mail contenu : %s\n", mail_contenu);

		/*Boucle de transparence pour transformer les "#" en "\n", opération inverse à faire coté reception*/
		for(i=0 ; i < strlen(mail_contenu); i++){
			if(mail_contenu[i] == '#'){
				mail_contenu[i] = remplacement;
			}
		}
		mail_contenu[strlen(mail_contenu)-2] = 0;

		/*Affichage des différents éléments du mail*/
		printf("\n");
		printf("%sExpéditeur :%s %s\n", GREY, DEFAULT_COLOR, mail_expediteur);
		printf("%sObjet :%s %s\n", GREY, DEFAULT_COLOR, mail_objet);
		printf("%sContenu :%s \n", GREY, DEFAULT_COLOR);
		printf("%s\n", mail_contenu);
		printf("\n");
		printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
		free(message);
		free(mail_expediteur);
		free(mail_objet);
		free(mail_contenu);
		return 0;
	} else {
		/*Gestion d'erreur si c'est pas "Reply/101$*"*/
		if (rep == 404){
			printf("\n");
			printf("Erreur lors de la lecture, le message n'a pas pu etre lu.\n");
			printf("\n");
			printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
			return 1;
		} else {
			printf("\n");
			printf("Erreur inconnue.\n");
			printf("\n");
			printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
			return 1;
		}
	}
	return 0;	/*La fonction retourne 0 si elle s'execute correctement*/
}


/*__________________________________Suppression Message________________________________________*/
int Delete(){
	/*Déclaration des variables*/
	char *message = calloc(50, sizeof (char));
	int num_message;	/*Permet de stocker le numéro du message*/
	char requete[20];	/*Permet de stocker la requête complete concatené dans un tableau de 5000 caractères'*/
	char confirmation; /*Permet de stocker la confirmation de suppression de l'Utilisateur*/
	int rep = 0;	/*Permet de stocker le type de réponse du serveur*/


	/*En-tête menu Suppression message*/
	printf("***************************** %sMENU%s ****************************\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sTMP Messagerie%s                       *\n", BLUE, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sSuppression mail%s                     *\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FFSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Récupération du nombre de messages à effacer*/
	do {
		printf("Quel mail voulez-vous effacez :\n");
		if (scanf("%d", &num_message) != 1){	/*Récupération du numéro de message et erreur
																					si c'est pas un entier*/
		  printf("\n");
			printf("Veuillez saisir un entier.\n");
			printf("\n");
			printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
			return 1;	/*On sort de la fonction si c'est pas un entier*/
		}

		printf("\n");
		printf("%sEtes-vous sûr de bien vouloir supprimer le mail n°%d ? (Y/n)%s\n", RED, num_message, DEFAULT_COLOR); /*On demande
																																													confirmation
																																													pour la suppression
																																													du mail*/
		FreeBuffer();
		scanf("%1c", &confirmation);	/*On stocke la réponse dans confirmation*/
		printf("\n");
	} while (confirmation != 'Y'); /*On refait toute la boucle si l'utilisateur ne confirme pas*/

	/*Concatenation des différents éléments*/
	sprintf(requete, "Delete/%d$*\n", num_message);	/*On concatene le nombre de message avec la requête
																									pour avoir la syntaxe "Delete/nombredemessage$*"*/

	/*Test de l'émission de l'envoie de la requete*/
	if(Emission(requete)!=1) {
		printf("Erreur lors de l'émission du fichier à effacer.\n");
		return 1;
	}

	/*Réception de la réponse du serveur*/
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message,"Reply/%d$*",&rep);	/*On extrait le paramètre de la reponse reçu,
																			qui correspond à l'état de la suppression du mail*/
	// free(message);	/*Libération de la mémoire*/
	if(rep == 101){
		printf("Votre message a bien été supprimé.\n");
		printf("\n");
		printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
		free(message);
		return 0;
	} else {
		if (rep == 505){
			printf("Erreur lors de la suppression, le message n'a pas pu etre effacé.\n");
			printf("\n");
			printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
			free(message);
			return 1;
		} else {
			printf("Erreur inconnue.\n");
			printf("\n");
			printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
			free(message);
			return 1;
		}
	}
	free(message);
	return 0;	/*La fonction retourne 0 si elle s'execute correctement*/
}

/*__________________________________Envoie d'un message______________________________________*/

int Send(){
	/*Déclaration des variables*/
	char *message = calloc(50, sizeof (char));
	char mail_destinataire[30]; /*Permet récupérer le destinataire du mail pour l'envoyer au serveur*/
	char mail_objet[100];	/*Permet récupérer l'objet du mail pour l'envoyer au serveur*/
	char *mail_contenu = calloc(5000, sizeof (char)); /*Permet récupérer le contenu du mail
																										pour l'envoyer au serveur, alloue 5000 cases mémoire
																										initialisées avec '\0' pour éviter des problèmes de
																										buffer et segmentation*/
	char requete[5010];	/*Permet de stocker la requête complete concatené dans un tableau de 5010 caractères'*/
	int rep;	/*Permet de stocker le type de réponse du serveur*/
	char remplacement = '#'; /*Permet de stocker un caractère pour remplacer les "\n" contenu dans le contenu du mail*/
	int i = 0; /*Variable pour la boucle do while pour le contenu du mail


	/*En-tête menu nombre de message*/
	printf("***************************** %sMENU%s ****************************\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                        %sTMP Messagerie%s                       *\n", BLUE, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                      %sEnvoie d'un message%s                    *\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FFSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Récupération du destinataire du message à envoyer*/
	printf("Veuillez rentrer l'adresse de votre destinataire :\n");
	FreeBuffer();
	fgets(mail_destinataire, 30, stdin);	/*On récupère la saisie du clavier qui est
																				le nombre de message dans la variable "num_message"*/
	mail_destinataire[strlen(mail_destinataire)-1] = '\0'; /*Suppression du "\n" à la fin*/


	/*Concatenation des différents éléments*/
	sprintf(requete, "Send/%s$*\n", mail_destinataire);	/*On concatene le nombre de message avec la requête
																											pour avoir la syntaxe "Mail/identifiant$*"*/

	/*Test de l'émission de l'envoie de la requete*/
	if(Emission(requete)!=1) {
		printf("Erreur lors de l'émission du destinataire.\n");
		return 1;
	}

	printf("\n");	/*Retour à la ligne pour un affichage plus clair*/

	/*Réception de la réponse du serveur*/
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message, "Reply/%d$*", &rep);	/*On extrait le paramètre de la reponse reçu,
																			qui correspond à l'état de reception du mail*/
	if (rep == 303){ /*Si le destinataire n'est pas existant sur le serveur, on sort de la fonction*/
		printf("\n");
		printf("Destinataire incorrecte, veuillez renseignez un destinataire existant.\n");
		printf("\n");
		printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
		free(message);
		return 1;
	} else if (rep == 101){ /*Si le destinataire existe bien sur le serveur*/
		/*Récupération de l'objet du message à envoyer*/
		printf("Veuillez rentrer l'objet de votre mail :\n");
		fgets(mail_objet, 100, stdin);	/*On récupère la saisie du clavier qui est le nombre de
		message dans la variable "num_message"*/
		mail_objet[strlen(mail_objet)-1] = '\0'; /*Suppression du "\n" à la fin*/

		/*Concatenation des différents éléments*/
		sprintf(requete, "Send/%s$*\n", mail_objet);	/*On concatene le nombre de message avec la requête
		pour avoir la syntaxe "Mail/objet$*"*/

		/*Test de l'émission de l'envoie de la requete*/
		if(Emission(requete)!=1) {
			printf("Erreur lors de l'émission du destinataire.\n");
			return 1;
		}

		printf("\n");	/*Retour à la ligne pour un affichage plus clair*/

		/*Récupération du contenu du message à envoyer*/
		printf("Contenu de votre mail (Terminez par \"ECHAP\" puis \"Entrée\" pour envoyer votre mail) :\n");

		/*Boucle do while pour récupérer tous le contenu du mail jusqu'au caractère ASCII 27 correpondant à ECHAP"*/
		do{
			mail_contenu[i] = fgetc(stdin);
			if (mail_contenu[i] == 27)
			break;
			i++;
		} while (1);

		/*Boucle de transparence pour transformer les "\n" en "#", opération inverse à faire coté reception*/
		for(i=0 ; i < strlen(mail_contenu); i++){
			if(mail_contenu[i] == '\n'){
				mail_contenu[i] = remplacement;
			}
		}

		/*Concatenation des différents éléments*/
		sprintf(requete, "Send/%s$*\n", mail_contenu);	/*On concatene le nombre de message avec la requête
		pour avoir la syntaxe "Mail/contenu$*"*/

		/*Test de l'émission de l'envoie de la requete*/
		if(Emission(requete)!=1) {
			printf("Erreur lors de l'émission du destinataire.\n");
			return 1;
		}

		/*Réception de la réponse du serveur*/
		message = Reception();	/*On stocke la reception dans la variable message*/
		sscanf(message, "Reply/%d$*", &rep);	/*On extrait le paramètre de la reponse reçu,
		qui correspond à l'état de reception du mail*/
		//free(message);	/*Libération de la mémoire*/
		if(rep == 101){
			printf("\n");
			printf("Votre message a été envoyé.\n");
			printf("\n");
			printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
			free(message);
			free(mail_contenu);
			return 0;
		} else {
			printf("\n");
			printf("Erreur inconnue.\n");
			printf("\n");
			printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
			free(message);
			free(mail_contenu);
			return 1;
		}
	}
	free(message);
	free(mail_contenu);
	return 0;	/*La fonction retourne 0 si elle s'execute correctement*/
}

/*______________________________Fonction nombres de messages_________________________________*/
int Inbox(){
	/*Déclaration des variables*/
	char *message = (char *) calloc(50, sizeof (char));
	int num = 0; /*Permet de stocker le nombre de message*/

	/*En-tête menu nombre de message*/
	printf("***************************** MENU ****************************\n");
	printf("*                                                             *\n");
	printf("*                        %sTMP Messagerie%s                       *\n", BLUE, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                      %sNombres de messages%s                    *\n", GREEN, DEFAULT_COLOR);
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FFSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Envoie de la requête "Inbox$*" pour demander le nombre de message sur le serveur*/
	/*Test de l'émission de l'envoie de la requete*/
	if(Emission("Inbox/$*\n")!=1) {
		printf("Erreur d'emission lors de l'envoie de Inbox.\n");	/*Affichage message d'erreur*/
		return 1;
	}

	/*Réception de la réponse du serveur*/
	bzero(message, 50);
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message,"Number/%d$*",&num); /*On extrait le paramètre de la reponse reçu,
																								qui correspond au nombre de messages*/


	/*Test si il y a des messages présents sur le serveur*/
	if(num == 0) {
		printf("Vous n'avez pas de messages dans votre boite de reception.\n");	/*Si il n'a pas de message présent sur le serveur,
																							l'utilisateur est averti*/
		printf("\n");
		printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
		return 1;
	} else if (num == 1) {
			printf("Vous avez %d message dans votre boite de reception.\n", num); /*Si l'utilisateur a des messages,
																							son nombre de message est affiché*/
			printf("\n");
			printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
	} else {
		printf("Vous avez %d messages dans votre boite de reception.\n", num); /*Si l'utilisateur a des messages,
																						son nombre de message est affiché*/
		printf("\n");
		printf("Appuyer sur \"Entrée\" pour revenir au Menu Principal.\n");
	}
	free(message);
	return num;	/*La fonction retourne 0 si elle s'execute correctement*/
}

/*_______________________Récupérer le nombre de message dans BAL_________________________*/
int Inbox_spy(){
	/*Déclaration des variables*/
	char *message = (char *) calloc(50, sizeof (char));
	int num = 0; /*Permet de stocker le nombre de message*/
	/*Envoie de la requête "Inbox$*" pour demander le nombre de message sur le serveur*/
	/*Test de l'émission de l'envoie de la requete*/
	if(Emission("Inbox/$*\n")!=1) {
		printf("Erreur d'emission lors de l'envoie de Inbox.\n");	/*Affichage message d'erreur*/
		return 1;
	}

	/*Réception de la réponse du serveur*/
	bzero(message, 50);
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message,"Number/%d$*",&num); /*On extrait le paramètre de la reponse reçu,
																								qui correspond au nombre de messages*/
	free(message);
	return num;	/*La fonction retourne 0 si elle s'execute correctement*/
}

/*____________________________________Déconnexion________________________________________*/
int Disconnect(){
	int choix = 0;
	do {
		/*En-tête menu déconnexion*/
		printf("***************************** %sMENU%s ****************************\n", GREEN, DEFAULT_COLOR);
		printf("*                                                             *\n");
	  printf("*                        %sTMP Messagerie%s                       *\n", BLUE, DEFAULT_COLOR);
		printf("*                                                             *\n");
		printf("*                          %sDéconnexion%s                        *\n", GREEN, DEFAULT_COLOR);
		printf("*                                                             *\n");
		printf("*                                                             *\n");
		printf("***************************************************************\n");
		printf("*************** %s1 - Pour confirmer votre déconnexion%s **********\n", RED, DEFAULT_COLOR);
		printf("*************** %s2 - Pour revenir au menu précédent%s  ***********\n", YELLOW, DEFAULT_COLOR);
		printf("***************************************************************\n");
		printf("*** Copyright FFSSSD ************************** Version 1.0 ***\n");
		printf("\n");

		choix = Choix(); /*Récupération du choix de l'utilisateur*/
		if (choix == 1){	/*Si son choix est 1, on ferme la connexion avec le serveur*/
			Emission("Disconnect/$*\n");
			Terminaison();
			printf("Vous êtes maintenant déconnecté\n");
			return 0;
		} else if (choix == 2){	/*Si son choix est 2, on retourne 1 pour revenir au menu précédent*/
			return 1;
		} else {
			printf("Veuillez renseignez un choix valide.\n");
		}
	} while(choix != 1 && choix != 2);
	return 0;	/*La fonction retourne 0 si elle s'execute correctement*/
}
