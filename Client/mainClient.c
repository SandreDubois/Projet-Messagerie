#include <stdio.h>
#include <stdlib.h>
#include "client.h"

int main() {
	char *message = NULL;

	if(Initialisation("localhost") != 1) {
		printf("Erreur d'initialisation\n");
		return 1;
	}

/*	message = Reception();
	if(message != NULL) {
		printf("J'ai recu: %s\n", message);
		free(message);
	} else {
		printf("Erreur de reception\n");
		return 1;
	}

	message = Reception();
	if(message != NULL) {
		printf("J'ai recu: %s\n", message);
		free(message);
	} else {
		printf("Erreur de reception\n");
		return 1;
	}

	message = Reception();
	if(message != NULL) {
		printf("J'ai recu: %s\n", message);
		free(message);
	} else {
		printf("Erreur de reception\n");
		return 1;
	}*/

/*________________________________________Notre Partie__________________________________________*/

	system("clear");
	/*Connexion au serveur*/
	while (Connexion());

	/*Demande du nombre de messages*/
	if (Inbox()){
		return -1;
	}

	/*Suppression d'un message*/
	if (Delete()){
		return -1;
	}

	/*Deconnexion*/
	Deconnexion();

	return 0;
}
