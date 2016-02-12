#include <stdio.h>
#include <stdlib.h>
#include "client.h"

int main() {
	char *message = NULL;

	if(Initialisation("localhost") != 1) {
		printf("Erreur d'initialisation\n");
		return 1;
	}

/*________________________________________Notre Partie__________________________________________*/

	system("clear");
	int choix = 0;
	int retourAuthentification = 1;
	int retourDeconnexion = 1;
	int retourDelete = 1;
	int retourInbox = 1;
	int retourRead = 1;
	int retourSend = 1;

 	/*Boucle pour l'authentification du client*/
	do {
		system("clear");
		/*Affichage du menu de connexion*/
		Menu_Authentification();
		choix = Choix();
		system("clear");
		switch (choix) {
			case 1:
				/*Connexion au serveur*/
				retourAuthentification = Authentification();
				break;

			case 2:
				/*Deconnexion*/
				retourDeconnexion = Deconnexion();
				if (retourDeconnexion == 0){
					return 0;
				}
				break;

			default :
				printf("Veuillez renseignez un choix valide.\n");
		}
		printf("Redirection dans 5 secondes. Veuillez patientez.\n");
		system("sleep 5");
	} while(retourDeconnexion != 0 && retourAuthentification != 0);

	/*Boucle principale pour l'éxecution de toutes les fonctions*/
	if (retourAuthentification != 1){
		do {
			system("clear");
			Menu_Principal();
			choix = Choix();
			system("clear");
			switch (choix) {
				case 1:
					/*Appel de la fonction Lecture d'un mail*/
					retourRead = Read();
					break;

				case 2:
					/*Appel de la fonction Suppression d'un message*/
					retourDelete = Delete();
					break;

				case 3:
					/*Appel de la fonction Ecriture d'un message*/
					//retourSend = Send();
					break;

				case 4:
					/*Appel de la fonction Nombre de messages*/
					retourInbox = Inbox();
					break;

				case 5:
					/*Appel de la fonction Déconnexion*/
					retourDeconnexion = Deconnexion();
					if (retourDeconnexion == 0){
						return 0;
					}
					break;

				default:
					printf("Veuillez renseignez un choix valide.\n");
			}
			printf("Redirection dans 5 secondes. Veuillez patientez.\n");
			system("sleep 5");
		} while(retourDelete !=0 && retourInbox != 0 && retourDeconnexion != 0 /*&& retourSend != 0 && retourRead != 0*/);
	}
	return 0;
}
