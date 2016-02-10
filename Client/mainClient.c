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
<<<<<<< HEAD
	/*Connexion au serveur*/
	while (Connexion() != 0);

	/*Demande du nombre de messages*/
	if (Inbox() != 0){
=======
	int choix = 0;
	int retourConnexion = 1;

 	/*Affichage du menu de connexion*/
	do {
		system("clear");
		Menu_Connexion();
		choix = Choix();
		system("clear");
		switch (choix) {
			case 1:
				/*Connexion au serveur*/
				retourConnexion = Connexion();
				break;

			case 2:
				/*Deconnexion*/
				Deconnexion();
				break;
		}
		printf("Redirection dans 5 secondes. Veuillez patientez.\n");
		system("sleep 5");
	} while(choix != 2 && retourConnexion != 0);

	Menu_Principal();



	/*Demande du nombre de messages*/
	if (Inbox()){
>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
		return -1;
	}

	/*Suppression d'un message*/
<<<<<<< HEAD
	if (Delete() != 0){
=======
	if (Delete()){
>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
		return -1;
	}

	/*Deconnexion*/
	Deconnexion();

	return 0;
}
