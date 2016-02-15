#include <stdio.h>
#include <stdlib.h>
#include "client.h"

int main() {
	char *message = NULL; /*Déclaration et initialisation d'un pointeur message
												qui contiendra les messages envoyé par le serveur*/

	/*Connexion entre le client et le serveur*/
	if(Initialisation("localhost") != 1) {
		printf("Erreur d'initialisation\n");
		return 1;
	}

/*________________________________________Notre Partie__________________________________________*/

	system("clear");	/*On efface le terminal pour un affichage plus clair*/

	/*Déclaration et initialisation des variables*/
	int choix = 0;	/*Contiendra le choix de l'utilisateur pour les menus*/
	int retourAuthentification = 1;	/*Contiendra le retour de la fonction authentification*/
	int retourDisconnect = 1;	/*Contiendra le retour de la fonction Disconnect*/
	int retourConsult = 1; /*Contiendra le retour de la fonction Consult*/
	int retourDelete = 1;	/*Contiendra le retour de la fonction Delete*/
	int retourInbox = 1;	/*Contiendra le retour de la fonction Inbox*/
	int retourRead = 1;	/*Contiendra le retour de la fonction Read*/
	int retourSend = 1;	/*Contiendra le retour de la fonction Send*/

 	/*Boucle pour l'authentification du client*/
	do {
		system("clear");
		Menu_Authentification();	/*Affichage du menu de connexion*/
		choix = Choix(); /*Recupération du choix de l'utilisateur que l'on stocke dans la
										 variable choix*/
		system("clear");

		/*Boucle pour le choix de la fonction*/
		switch (choix) {
			case 1:
				/*Connexion au serveur*/
				retourAuthentification = Authentification();
				break;

			case 2:
				/*Deconnexion*/
				retourDisconnect = Disconnect();
				if (retourDisconnect == 0){	/*Si il choisit la déconnexion, cela retourne 0
																			donc on sort de la boucle*/
					return 0;
				}
				break;

			default : /*Si choix n'est ni 1, ni 2, affiche un message d'erreur et recommence
								la boucle*/
				printf("Veuillez renseignez un choix valide.\n");
		}
		system("clear");
		printf("Chargement en cours, veuillez patientez.\n");
		system("sleep 1");
	} while(retourDisconnect != 0 && retourAuthentification != 0); /*On refait cette boucle tant que
																																	la fonction Disconnect retourne 1,
																																	c'est à dire que l'utilisateur veuille revenir au menu
																																	précedent et tant que l'authentification se passe
																																	mal.*/

	/*Boucle principale pour l'éxecution de toutes les fonctions*/
	if (retourAuthentification != 1){ /*On rentre dans la boucle que si l'authentification est correcte*/
		do {
			system("clear");
			Menu_Principal();	/*Affichage du menu de principal*/
			choix = Choix();	/*Recupération du choix de l'utilisateur que l'on stocke dans la
											 variable choix*/
			system("clear");
			switch (choix) {
				case 1:
					/*Appel de la fonction Lecture d'un mail*/
					retourConsult = Consult();
					if (RetourMenuPrecedent() == 0){ /*Une fois la fonction est fini correctement,
																					 on execute la fonction RetourMenuPrincipal*/
						break;
					}

				case 2:
					/*Appel de la fonction Lecture d'un mail*/
					retourRead = Read();
					if (RetourMenuPrecedent() == 0){ /*Une fois la fonction est fini correctement,
																					 on execute la fonction RetourMenuPrincipal*/
						break;
					}

				case 3:
					/*Appel de la fonction Suppression d'un message*/
					retourDelete = Delete();
					if (RetourMenuPrecedent() == 0){	/*Une fois la fonction est fini correctement,
																					 on execute la fonction RetourMenuPrincipal*/
						break;
					}

				case 4:
					/*Appel de la fonction Ecriture d'un message*/
					retourSend = Send();
					if (RetourMenuPrecedent() == 0){	/*Une fois la fonction est fini correctement,
																					 on execute la fonction RetourMenuPrincipal*/
						break;
					}

				case 5:
					/*Appel de la fonction Nombre de messages*/
					retourInbox = Inbox();
					if (RetourMenuPrecedent() == 0){	/*Une fois la fonction est fini correctement,
																					 on execute la fonction RetourMenuPrincipal*/
						break;
					}

				case 6:
					/*Appel de la fonction Déconnexion*/
					retourDisconnect = Disconnect();
					if (retourDisconnect == 0){	/*Si Deconnexion se passe bien, on retourne 0
																				pour sortir de la boucle*/
						return 0;
					}
					break;

				default:	/*Si choix différent de ceux-ci dessus, affiche un message d'erreur et recommence
									la boucle*/
					printf("Veuillez renseignez un choix valide.\n");
			}
			system("clear");
			printf("Chargement en cours, veuillez patientez.\n");
			system("sleep 1");
		} while(retourDisconnect != 0); /*On sort pas de cette boucle tant que l'utilisateur ne
																		choisit pas déconnexion et confirme la déconnexion
																		ainsi la fonction Déconnxion retourne 1*/
	}
	return 0;
}
