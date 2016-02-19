#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serveur.h"

int main(){

//Déclaration des variables
	char *message = (char *) calloc(5000,sizeof(char));
	char *chemin = (char *) calloc(50,sizeof(char));
	char *login = (char *) calloc(30,sizeof(char));
	char *paswd = (char *) calloc(30,sizeof(char));
	char *nature_requete = (char *) calloc(30,sizeof(char));
	char *Authentification = "Authentification";
	char *Consult = "Consult";
	char *Read = "Read";
	char *Delete = "Delete";
	char *Send = "Send";
	char *Inbox = "Inbox";
	char *Disconnect = "Disconnect";
	char number[20];
	int resultat;
	char num_message[3];
	int authentification_incorrect = 0;

	system("clear");
	initialisation();

	while(1) {
		int fini = 0;

		attenteclient();

		while(!fini) {
			do {
				bzero(message, 50); /*Remet à zero le tableau message*/
				receptionbinaire(message,50);
				if(message != NULL) {
					printf("J'ai recu: %s\n", message);

					int resultat = analyser(message);

					extraction(strlen(message), message, nature_requete,'/');
					printf("Nature requete : %s\n", nature_requete);

					resultat = authentification(message,login,paswd);
					printf("[Authentification] : login : %s, password : %s\n",login, paswd);
					if (resultat < 0)
						printf("Erreur lors de l'extration des paramètres\n");

					if (!ouverture_id(login)){
						printf("Identifiant correct\n");
						int resmdp = lecture_paswd(login,paswd);
						if (resmdp != 0){
							authentification_incorrect = 1;
						} else {
							authentification_incorrect = 0;
						}
						if (!resmdp){
							printf("Mot de passe correct\n");
							//se placer dans le dossier de l'utilisateur
							emission("Reply/101$*\n");
						}
						else{
							printf("Mot de passe incorrect !\n");
							emission("Reply/202$*\n");
						}
					}
					else{
						printf("Identifiant incorrect !\n");
						emission("Reply/202$*\n");
					}
				} else {
						printf("Erreur de reception\n");
						return 1;
				}
				printf("Retour de lecture password : %d\n", authentification_incorrect);
				if (authentification_incorrect == 0) {
					do {
						//attenteclient();
						bzero(message, 50); /*Remet à zero le tableau message*/
						receptionbinaire(message,50);
						if(message != NULL) {
							/*Extraction du nom de la fonction dans la requete*/
							printf("J'ai recu: %s", message);
							extraction(strlen(message), message, nature_requete,'/'); /*On extrait la fonction Delete*/
							printf("Nature requete : %s\n", nature_requete); /*On affiche que la fonction soit bien extraite*/

							/*Boucle if principale qui execute la fonction en fonction de la requête envoyée*/
							if (!strcmp(nature_requete, Consult)){	/*Fonction Consult*/
								sprintf(chemin, "./users/%s", login);
								content(chemin);
							} else if (!strcmp(nature_requete, Read)){ /*Fonction Read*/
								sprintf(chemin, "./users/%s", login);
								sscanf(message, "Read/%s$*", &num_message);
								num_message[strlen(num_message)-2] = 0;
								printf("Chemin : %s, Message n° : %s\n",chemin, num_message);
								mail(chemin, num_message);
							} else if (!strcmp(nature_requete, Delete)){ /*Fonction Delete*/
								sprintf(chemin, "./users/%s", login);
								sscanf(message, "Delete/%s$*", &num_message);
								num_message[strlen(num_message)-2] = 0;
								printf("Chemin : %s, Message n° : %s\n",chemin, num_message);
								if (supprimer_message(chemin, num_message) == 0)
									emission("Reply/101$*\n");
								else
									emission("Reply/505$*\n");
							} else if (!strcmp(nature_requete, Send)){ /*Fonction Send*/
								if(stockage_message(message, login) == 0)
									emission("Reply/101$*\n");
								else
									emission("Reply/303$*\n");
							} else if (!strcmp(nature_requete, Inbox)){	/*Fonction Inbox*/
								sprintf(chemin, "./users/%s", login);
								printf("Chemin :%s\n",chemin);
								if (nombre_msg_boite_mail(chemin) != -1){
									int nbr = nombre_msg_boite_mail(chemin);
									sprintf(number, "Number/%d$*\n", nbr);
									emission(number);
								}
							} else if (!strcmp(nature_requete, Disconnect)){
								return 1;
							}
						} else {
							printf("Erreur de reception\n");
							return 1;
						}
					} while (1); /*Condition boucle du menu principal*/
				}
			} while (!authentification_incorrect); /*Condition boucle du menu d'authentification*/

			//fini = 1;
		}
		terminaisonclient();
	}
	return 0;
}


/* Il manque

envoi liste des enêtes des messages
création d'un mail
envoi d'un mail complet au client
*/
