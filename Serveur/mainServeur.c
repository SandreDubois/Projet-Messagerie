#include <stdio.h>
#include <stdlib.h>
#include "serveur.h"

int main() {
  	//Variables
	char *message = NULL;
	char reponse[20];

	Initialisation();

	while(1) {
		int fini = 0;

		AttenteClient();

		while(!fini) {
			message = Reception();

			if(message != NULL) {
				printf("J'ai recu: %s\n", message);
				Emission("Reply/101$*\n");
				free(message);

				if(Emission("Test de message serveur.\n")!=1) {
					printf("Erreur d'emission\n");
				}
			} else {
				fini = 1;
			}
		}

		TerminaisonClient();
	}

	return 0;
}
