
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
			printf("Erreur lors de la suppression, le message n'a pas pu etre effacé.\n");
			return 1;
		} else {
			printf("Erreur inconnue.\n");
			return 1;
		}
	}
	return 0;
}

int Deconnexion(){
	Terminaison();
	printf("Vous êtes maintenant déconnecté\n");
	return 0;
}
