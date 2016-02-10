
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
	if(strstr(message, "\n") == NULL) {
		fprintf(stderr, "Emission, Le message n'est pas termine par \\n.\n");
	}
	int taille = strlen(message);
	if (send(socketClient, message, taille,0) == -1) {
        perror("Emission, probleme lors du send.");
        return 0;
	}
	printf("Emission de %d caracteres.\n", taille+1);
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
<<<<<<< HEAD
}

/*________________________________________Notre Partie__________________________________________*/
typedef struct{
	char adresse_client[30];
	char mdp_client[20];
} identifiants;

/* Connexion au serveur*/
int Connexion(){
	identifiants id;
	char *message = NULL;
	printf("Veuillez saisir votre adresse mail :\n");
	fgets(id.adresse_client, 30, stdin);
	if(Emission(id.adresse_client)!=1) {
		printf("Erreur lors de l'émission de l'adresse mail.\n");
		return 1;
	}

	printf("Veuillez saisir votre mot de passe :\n");
	fgets(id.mdp_client, 20, stdin);
	if(Emission(id.mdp_client)!=1) {
		printf("Erreur lors de l'émission de l'adresse mail du mot de passe.\n");
		return 1;
	}
	message = Reception();
	if(!strcmp(message,"101")) {
		printf("Authentification Réussie.\n");
		return 0;
	} else {
		if (!strcmp(message,"202")){
			printf("Erreur d'authentification, votre adresse mail et le mot de passe ne correspondent pas.\n");
			return 1;
		} else {
			printf("Erreur inconnue.\n");
			return 1;
		}
	}
	return 0;
}

int Inbox(){
	char *message = NULL;
	if(Emission("Inbox\n")!=1) {
		printf("Erreur d'emission lors de l'envoie de Inbox.\n");
		return 1;
	}
	message = Reception();
	if(message != NULL) {
		printf("Vous avez %s messages.\n", message);
		free(message);
	} else {
		printf("Erreur de lors de la reception du nombre de message.\n");
		return 1;
	}
	return 0;
}

int Delete(){
	char *message = NULL;
	char num_message[3];
	printf("Quel message voulez-vous effacez :\n");
	fgets(num_message, 3, stdin);
	if(Emission(num_message)!=1) {
		printf("Erreur lors de l'émission du fichier à effacer.\n");
		return 1;
	}
	message = Reception();
	if(!strcmp(message,"101")) {
		printf("Votre message a bien été supprimé.\n");
		return 0;
	} else {
		if (!strcmp(message,"505")){
=======


}

/*___________________________________________DEBUT______________________________________________*/

/*Fonction pour vider le buffer*/
void FreeBuffer()
{
    int c = 0;
    while (c != '\n' && c != EOF)
    {
        c = getchar();
    }
}

/*______________________________Affichage menu connexion_______________________________________*/
void Menu_Authentification(){
	printf("***************************** MENU ****************************\n");
	printf("*                                                             *\n");
	printf("*                        TMP Messagerie                       *\n");
	printf("*                                                             *\n");
	printf("*                    Menu d'Authentification                  *\n");
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
  printf("*************** 1 - Pour vous authentifier ********************\n");
  printf("*************** 2 - Pour vous déconnecter  ********************\n");
	printf("***************************************************************\n");
	printf("*                                                             *\n");
	printf("* Projet de Frederic FERRERA, Saidaran SARMA et Sandre DUBOIS *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FRSSSD ************************** Version 1.0 ***\n");
	printf("\n");
}

/*____________________________________Connexion au serveur______________________________________*/
int Authentification(){
	/*Déclaration des variables*/
	char *message = NULL; /*Permet de stocker le message reçu du serveur*/
	int rep;	/*Permet de stocker le type de réponse du serveur*/
	char adresse_client[30];	/*Permet de stocker l'adresse mail du client dans un tableau de 30 caractères'*/
	char mdp_client[20];	/*Permet de stocker le mot de passe du client dans un tableau de 20 caractères'*/
	char requete[5000];	/*Permet de stocker la requête complete concatené dans un tableau de 5000 caractères'*/

	/*En-tête menu authentification*/
	printf("***************************** MENU ****************************\n");
	printf("*                                                             *\n");
	printf("*                          Messagerie                         *\n");
	printf("*                                                             *\n");
	printf("*                    Menu d'Authentification                  *\n");
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FRSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Récupération de l'adresse mail*/
	printf("Veuillez saisir votre adresse mail :\n");
	FreeBuffer(); /*On vide le buffer, pour eviter toutes erreurs ultérieure*/
	fgets(adresse_client, 30, stdin);	/*On récupère la saisie du clavier qui est l'adresse mail dans la variable "adresse_client"*/
	adresse_client[strlen(adresse_client)-1] = '\0';	/*On retire le "\n" à la requête  car fgets met automatiquement un "\n" à la fin*/

	printf("\n");	/*Retour à la ligne pour un affichage plus clair*/

	/*Récupération du mot de passe*/
	printf("Veuillez saisir votre mot de passe :\n");
	fgets(mdp_client, 20, stdin);	/*On récupère la saisie du clavier qui est le mot de passe dans la variable "mdp_client"*/
	mdp_client[strlen(mdp_client)-1] = '\0'; /*Suppression du "\n" à la fin*/

	/*Concatenation des différents éléments*/
	sprintf(requete, "Authentification/%s/%s$*\n", adresse_client, mdp_client);	/*On concatene les identifiants et mot de passe avec la requête
																																								pour avoir la syntaxe "Authentification/identifiant/motdepasse$*"*/
	/*printf("%s\n", requete);	/*Retour de la requête complete pour test*/

	/*Test de l'émission de la requete*/
	if(Emission(requete)!=1) { /*On test si l'envoie de la requete c'est faite correctement sinon erreur lors de l'émission*/
		printf("Erreur lors de l'émission de l'adresse mail du mot de passe.\n"); /*Affichage message d'erreur*/
		return 1;
	}

	/*Réception de la réponse du serveur*/
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message,"Reply/%d$*",&rep); /*On extrait le paramètre de la reponse reçu, qui correspond à l'état de l'authentification*/

	/*Exploitation de la réponse du serveur*/
	if(rep == 101){	/*Si on reçoit un "Reply/101$*", alors authentification correcte*/
		printf("Authentification Réussie.\n");	/*Affichage message d'erreur*/
		return 0;
	} else {
		if (rep == 202){	/*Si on reçoit un "Reply/202$*", alors authentification échec*/
			printf("Erreur d'authentification, votre adresse mail et le mot de passe ne correspondent pas.\n");	/*Affichage message d'erreur*/
			return 1;
		} else { /*Sinon erreur inconnue*/
			printf("Erreur inconnue.\n");	/*Affichage message d'erreur*/
			return 1;
		}
	}
	return 0;	/*La fonction retourne 0 si elle s'execute correctement*/
}


/*________________________________Affichage menu principal______________________________________*/
void Menu_Principal(){
	printf("***************************** MENU ****************************\n");
	printf("*                                                             *\n");
	printf("*                        TMP Messagerie                       *\n");
	printf("*                                                             *\n");
	printf("*                        Menu Principal                       *\n");
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
  printf("************ 1 - Pour consulter votre messagerie   ************\n");
  printf("************ 2 - Pour lire un mail                 ************\n");
	printf("************ 3 - Pour supprimer un mail            ************\n");
	printf("************ 4 - Pour envoyer un mail              ************\n");
	printf("************ 5 - Pour obtenir le nombre de message ************\n");
	printf("************ 6 - Pour déconnecter                  ************\n");
	printf("***************************************************************\n");
	printf("*** Copyright FRSSSD ************************** Version 1.0 ***\n");
	printf("\n");
}

/*___________________________Récupération du choix de l'utilisateur____________________________*/
int Choix(){
	int choix;
	scanf("%d", &choix);
	return choix;
}

/*________________________________Consultation des messages____________________________________*/

/*__________________________________Lecture d'un messages______________________________________*/
int Read(){
	/*Déclaration des variables*/
	char *message = NULL;	/*Permet de stocker le message reçu du serveur*/
	char num_message[3]; /*Permet de stocker le numéro du message dans un tableaude 3 caractère*/
	char requete[5000];	/*Permet de stocker la requête complete concatené dans un tableau de 5000 caractères'*/
	int rep;	/*Permet de stocker le type de réponse du serveur*/

	/*En-tête menu Suppression message*/
	printf("***************************** MENU ****************************\n");
	printf("*                                                             *\n");
	printf("*                        TMP Messagerie                       *\n");
	printf("*                                                             *\n");
	printf("*                        Lecture Message                      *\n");
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FRSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Récupération du nombre de messages à effacer*/
	printf("Quel message voulez-vous lire :\n");
	fgets(num_message, 3, stdin);	/*On récupère la saisie du clavier qui est le nombre de message dans la variable "num_message"*/

	/*Concatenation des différents éléments*/
	sprintf(requete, "Read/%s$*\n", num_message);	/*On concatene le nombre de message avec la requête
																									pour avoir la syntaxe "Delete/nombredemessage$*"*/

	/*Test de l'émission de l'envoie de la requete*/
	if(Emission(requete)!=1) {
		printf("Erreur lors de l'émission du fichier à lire.\n");
		return 1;
	}

	/*Réception de la réponse du serveur*/
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message,"Reply/%d$*",&rep);	/*On extrait le paramètre de la reponse reçu, qui correspond au nombre de messages*/
	if(rep == 101){
		//A FAIRE
	} else {
		if (rep == 404){
			printf("Erreur lors de la lecture, le message n'a pas pu etre lu.\n");
			return 1;
		} else {
			printf("Erreur inconnue.\n");
			return 1;
		}
	}
	return 0;
}


/*__________________________________Suppression Message________________________________________*/
int Delete(){
	/*Déclaration des variables*/
	char *message = NULL;	/*Permet de stocker le message reçu du serveur*/
	char num_message[3]; /*Permet de stocker le numéro du message dans un tableaude 3 caractère*/
	char requete[5000];	/*Permet de stocker la requête complete concatené dans un tableau de 5000 caractères'*/
	int rep;	/*Permet de stocker le type de réponse du serveur*/

	/*En-tête menu Suppression message*/
	printf("***************************** MENU ****************************\n");
	printf("*                                                             *\n");
	printf("*                        TMP Messagerie                       *\n");
	printf("*                                                             *\n");
	printf("*                      Suppression message                    *\n");
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FRSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Récupération du nombre de messages à effacer*/
	printf("Quel message voulez-vous effacez :\n");
	fgets(num_message, 3, stdin);	/*On récupère la saisie du clavier qui est le nombre de message dans la variable "num_message"*/

	/*Concatenation des différents éléments*/
	sprintf(requete, "Delete/%s$*\n", num_message);	/*On concatene le nombre de message avec la requête
																									pour avoir la syntaxe "Delete/nombredemessage$*"*/

	/*Test de l'émission de l'envoie de la requete*/
	if(Emission(requete)!=1) {
		printf("Erreur lors de l'émission du fichier à effacer.\n");
		return 1;
	}

	/*Réception de la réponse du serveur*/
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message,"Reply/%d$*",&rep);	/*On extrait le paramètre de la reponse reçu, qui correspond au nombre de messages*/
	if(rep == 101){
		printf("Votre message a bien été supprimé.\n");
		return 0;
	} else {
		if (rep == 505){
>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
			printf("Erreur lors de la suppression, le message n'a pas pu etre effacé.\n");
			return 1;
		} else {
			printf("Erreur inconnue.\n");
			return 1;
		}
	}
	return 0;
}

<<<<<<< HEAD
/*__________________________________Envoie d'un message______________________________________*/


/*______________________________Fonction nombres de messages_________________________________*/
int Inbox(){
	/*Déclaration des variables*/
	char *message = NULL;	/*Permet de stocker le message reçu du serveur*/
	int num; /*Permet de stocker le nombre de message*/

	/*En-tête menu nombre de message*/
	printf("***************************** MENU ****************************\n");
	printf("*                                                             *\n");
	printf("*                        TMP Messagerie                       *\n");
	printf("*                                                             *\n");
	printf("*                      Nombres de messages                    *\n");
	printf("*                                                             *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	printf("*** Copyright FRSSSD ************************** Version 1.0 ***\n");
	printf("\n");

	/*Envoie de la requête "Inbox$*" pour demander le nombre de message sur le serveur*/
	/*Test de l'émission de l'envoie de la requete*/
	if(Emission("Inbox$*\n")!=1) {
		printf("Erreur d'emission lors de l'envoie de Inbox.\n");	/*Affichage message d'erreur*/
		return 1;
	}

	/*Réception de la réponse du serveur*/
	message = Reception();	/*On stocke la reception dans la variable message*/
	sscanf(message,"Number/%d$*",&num);	/*On extrait le paramètre de la reponse reçu, qui correspond au nombre de messages*/

	/*Test si il y a des messages présents sur le serveur*/
	if(num != 0) {
		printf("Vous avez %s messages.\n", num); /*Si l'utilisateur a des messages, son nombre de message est affiché*/
		free(message);
	} else {
		printf("Vous n'avez pas de messages.\n");	/*Si il n'a pas de message présent sur le serveur, l'utilisateur est averti*/
		return 1;
	}
	return 0;	/*La fonction retourne 0 si elle s'execute correctement*/
}


/*____________________________________Déconnexion____________________________________________*/
=======
<<<<<<< HEAD
=======
/*____________________________________Déconnexion________________________________________*/
>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
>>>>>>> a463084667e83f53bd0f6255e26dd84e59a232b8
int Deconnexion(){
	int choix = 0;
	do {
		/*En-tête menu déconnexion*/
		printf("***************************** MENU ****************************\n");
		printf("*                                                             *\n");
	  printf("*                        TMP Messagerie                       *\n");
		printf("*                                                             *\n");
		printf("*                          Déconnexion                        *\n");
		printf("*                                                             *\n");
		printf("*                                                             *\n");
		printf("***************************************************************\n");
		printf("*************** 1 - Pour confirmer votre déconnexion **********\n");
		printf("*************** 2 - Pour revenir au menu précédent  ***********\n");
		printf("***************************************************************\n");
		printf("*** Copyright FRSSSD ************************** Version 1.0 ***\n");
		printf("\n");

		choix = Choix();
		if (choix == 1){
			Terminaison();
			printf("Vous êtes maintenant déconnecté\n");
			return 0;
		} else if (choix == 2){
			return 1;
		} else {
			printf("Veuillez renseignez un choix valide.\n");
		}
	} while(choix != 1 && choix != 2);
	return 0;
}
