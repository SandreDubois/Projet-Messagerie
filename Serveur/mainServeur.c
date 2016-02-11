#include <stdio.h>
#include <stdlib.h>
#include "serveur.h"

int main() {
		char *message = calloc(50,sizeof(char));		//Initialise le message à 0
	Initialisation();

	while(1) {
		int fini = 0;

		AttenteClient();

		while(!fini) {
			message = ReceptionBinaire(message,50);
			if(message != NULL) {
				int resultat = Analyser(message);

				switch (resultat){
					case -1:
						printf("La syntaxe est incorrecte : il manque une '*' après le '$'\n");
						break;//à finir

					case -2: printf("%s\n", );

					default:
				}
				printf("J'ai recu: %s\n", message);
				free(message);
			} else {
				printf("Erreur de r�ception\n");
				return 1;
			}
				fini = 1;
			}
		}

		TerminaisonClient();
	}

	return 0;
}
