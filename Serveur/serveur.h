#ifndef __SERVEUR_H__
#define __SERVEUR_H__

/* Initialisation.
 * Creation du serveur.
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int initialisation();

/* Initialisation.
 * Creation du serveur en pr�cisant le service ou num�ro de port.
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int initialisationavecservice(char *service);


/* Attends qu'un client se connecte.
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int attenteclient();

/* Recoit un message envoye par le client.
 * retourne le message ou NULL en cas d'erreur.
 * Note : il faut liberer la memoire apres traitement.
 */
char *reception();

/* Envoie un message au client.
 * Attention, le message doit etre termine par \n
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int emission(char *message);

/* Recoit des donnees envoyees par le client.
 * renvoie le nombre d'octets re�us, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
 */
int receptionbinaire(char *donnees, size_t tailleMax);

/* Envoie des donn�es au client en pr�cisant leur taille.
 * renvoie le nombre d'octets envoy�s, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
 */
int emissionbinaire(char *donnees, size_t taille);

/* Fonction permettant de prendre le message et d'ajouter la chaine
 * caractères $*.
 */
int analyser(char *requete_recue);

void freebuffer();

int format_message(char *message);

int extract_requete(char *message, char *requete_extrait);

int authentification(char* message,char* login_extrait, char* paswd_extrait);


int ouverture_id(char* login_recu);

int lecture_paswd(char *login, char *paswd);

int nombre_msg_boite_mail (char *chemin);

int supprimer_message (char chemin[256], char numero_message[3]);

int stockage_message(char *message, char *login);

int content(char chemin[256]);

int mail(char chemin[256], char numero_message[3]);

/* Ferme la connexion avec le client.
 */
void terminaisonclient();

/* Arrete le serveur.
 */
void terminaison();

#endif
