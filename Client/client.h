#ifndef __CLIENT_H__
#define __CLIENT_H__

/* Initialisation.
 * Connexion au serveur sur la machine donnee.
 * Utilisez localhost pour un fonctionnement local.
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int Initialisation(char *machine);

/* Initialisation.
 * Connexion au serveur sur la machine donnee et au service donne.
 * Utilisez localhost pour un fonctionnement local.
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int InitialisationAvecService(char *machine, char *service);

/* Recoit un message envoye par le serveur.
 * retourne le message ou NULL en cas d'erreur.
 * Note : il faut liberer la memoire apres traitement.
 */
char *Reception();

/* Envoie un message au serveur.
 * Attention, le message doit etre termine par \n
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int Emission(char *message);

/* Recoit des donnees envoyees par le serveur.
 * renvoie le nombre d'octets re�us, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
 */
int ReceptionBinaire(char *donnees, size_t tailleMax);

/* Envoie des donn�es au serveur en pr�cisant leur taille.
 * renvoie le nombre d'octets envoy�s, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
 */
int EmissionBinaire(char *donnees, size_t taille);

/* Ferme la connexion.*/
void Terminaison();

/*Fonction vider Buffer*/
void FreeBuffer();

/*Fonction pour revenir au menu principal*/
int RetourMenuPrecedent();
int RetourMenuPrecedent_2();

/*Fonction pour l'extraction de chaine de caractère en paramètre*/
int extraction(int lg_chaine, char *source, char *destination, char condition);

/*Fonction pour récupérer le choix du menu de l'utilisateur*/
int Choix();

/*Fonction affichage du menu d'Authentification*/
void Menu_Authentification();

/*Fonction d'Authentification*/
int Authentification();

/*Fonction affichage du menu principal*/
void Menu_Principal();

/*Fonction pour récupérer et afficher la liste des messages*/
int Consult(int nombre_message_boite_mail);

/*Fonction de lecture d'un mail*/
int Read();

/*Fonction de suppression d'un mail*/
int Delete();

/*Fonction d'envoie du Message*/
int Send();

/*Fonction pour récupérer le nombre de message*/
int Inbox();
int Inbox_spy();

/* Ferme la connexion.*/
int Deconnexion();

#endif
