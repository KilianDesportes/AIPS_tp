#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

void construire_message(char * message, char motif, int lg){
	int i;
	for(i = 0 ; i < lg ; i++){
		message[i] = motif;
	}
}

void afficher_message(char *message, int lg) {
	int i;
	printf("message construit : ");
	for (i=0;i<lg;i++){
		printf("%c", message[i]); 
	}
	printf("\n");
}

int main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = 10; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0 = Serveur, 1 = Client */
	int type_client = -1; /* 0 = Recepteur , 1 = Emetteur */
	int len_message = 30; /* Longueur du message, defaut : 30 */
	int protocole = 0; /* 0 = TCP, 1 = UDP */
	int nb_msg_modif = 0; /* 1 = modification, 0 = pas de modification */
	int nb_port = -1; /* Numéro de port */
	int nb_port_htons = -1; /* Numéro de port big endian */
	char * host_name; /* Nom d'hôte */
	int sock; /* Socket */
	char * proc; /* Nom du protocole */
	// Partie INITIALISATION DES PARAMETRES
	while ((c = getopt(argc, argv, "cersul:n:")) != -1) {
		switch (c) {
		case 's':
			if (source == 1) {
				printf("usage: tsock [-c [-e|-r]|-s] [-u] [-n ##] [-l ##] [host] port\n");
				exit(1);
			}
			source = 0;
			break;
		case 'c':
			if (source == 0) {
				printf("usage: tsock [-c [-e|-r]|-s] [-u] [-n ##] [-l ##] [host] port\n");
				exit(1) ;
			}
			source = 1;
			host_name = argv[argc-2];
			break;
		case 'r':
			if (source == 0){
				printf("usage: tsock [-c [-e|-r]|-s] [-u] [-n ##] [-l ##] [host] port\n");
				exit(1) ;	
			}
			type_client == 0;
		case 'r':
			if (source == 0){
				printf("usage: tsock [-c [-e|-r]|-s] [-u] [-n ##] [-l ##] [host] port\n");
				exit(1) ;	
			}
			type_client == 1;
		case 'l':
			len_message = atoi(optarg);
			break;
		case 'u':
			protocole = 1;
			break;
		case 'n':
			nb_message = atoi(optarg);
			nb_msg_modif = 1;
			break;
		default:
			printf("usage: tsock [-c [-e|-r]|-s] [-u] [-n ##] [-l ##] [host] port\n");
			break;
		}
	}
	if (source == -1) {
		printf("usage: tsock [-c [-e|-r]|-s] [-u] [-n ##] [-l ##] [host] port\n");
		exit(1) ;
	}
	nb_port = atoi(argv[argc-1]);
	nb_port_htons = htons(nb_port);
	if (nb_port == -1){
		printf("usage: tsock [-c [-e|-r]|-s] [-u] [-n ##] [-l ##] [host] port\n");
		exit(1);
	}
	// Partie EMISSION DU MESSAGE
	if(protocole == 1){
		if((sock = socket(AF_INET,SOCK_DGRAM,0)) == -1){
			perror("Echec de création du socket\n");
			exit(1);
		}else{
			proc = "UDP";
		}
	}else{
		if((sock = socket(AF_INET,SOCK_STREAM,0)) == -1){
			perror("Echec de création du socket\n");
			exit(1);
		}else{
			proc = "TCP";
		}
	}


	// SERVEUR
	if(source == 0){
			
		
	}else if(type_client == 0){
	//CLIENT RECEPTEUR


	}else{
	//CLIENT EMETTEUR

	}
	return 0;
}