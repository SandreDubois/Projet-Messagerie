#ifndef __SERVEUR_H__
#define __SERVEUR_H__

/* Initialisation.
 * Creation du serveur.
<<<<<<< HEAD
 * renvoie 1 si �a c'est bien pass� 0 sinon
=======
 * renvoie 1 si �a c'est bien pass� 0 sinon
>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
 */
int Initialisation();

/* Initialisation.
<<<<<<< HEAD
 * Creation du serveur en pr�cisant le service ou num�ro de port.
 * renvoie 1 si �a c'est bien pass� 0 sinon
=======
 * Creation du serveur en pr�cisant le service ou num�ro de port.
 * renvoie 1 si �a c'est bien pass� 0 sinon
>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
 */
int InitialisationAvecService(char *service);


/* Attends qu'un client se connecte.
<<<<<<< HEAD
 * renvoie 1 si �a c'est bien pass� 0 sinon
=======
 * renvoie 1 si �a c'est bien pass� 0 sinon
>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
 */
int AttenteClient();

/* Recoit un message envoye par le client.
 * retourne le message ou NULL en cas d'erreur.
 * Note : il faut liberer la memoire apres traitement.
 */
char *Reception();

/* Envoie un message au client.
 * Attention, le message doit etre termine par \n
<<<<<<< HEAD
 * renvoie 1 si �a c'est bien pass� 0 sinon
=======
 * renvoie 1 si �a c'est bien pass� 0 sinon
>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
 */
int Emission(char *message);

/* Recoit des donnees envoyees par le client.
<<<<<<< HEAD
 * renvoie le nombre d'octets re�us, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
 */
int ReceptionBinaire(char *donnees, size_t tailleMax);

/* Envoie des donn�es au client en pr�cisant leur taille.
 * renvoie le nombre d'octets envoy�s, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
=======
 * renvoie le nombre d'octets re�us, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
 */
int ReceptionBinaire(char *donnees, size_t tailleMax);

/* Envoie des donn�es au client en pr�cisant leur taille.
 * renvoie le nombre d'octets envoy�s, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
 */
int EmissionBinaire(char *donnees, size_t taille);


/* Ferme la connexion avec le client.
 */
void TerminaisonClient();

/* Arrete le serveur.
 */
void Terminaison();

<<<<<<< HEAD
=======
int Connexion();

>>>>>>> fd03d90f6eac893d3acf58a099b3d609e7618746
#endif
