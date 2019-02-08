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
	printf("[");
	for (i=0;i<lg;i++){
		printf("%c", message[i]); 
	}
	printf("]\n");
}

int main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = 10; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	int len_message = 30; /* Longueur du message, defaut : 30 */
	int protocole = 0; /* 0 = TCP, 1 = UDP */
	int nb_port = -1; /* Numéro de port */
	int nb_port_htons = -1; /* Numéro de port big endian */
	char * host_name; /* Nom d'hôte */
	int sock; /* Socket */
	char * proc; /* Nom du protocole */
	// Partie INITIALISATION DES PARAMETRES
	while ((c = getopt(argc, argv, "psu")) != -1) {
		switch (c) {
		case 'p':
			if (source == 1) {
				printf("usage: tsock [-p|-s] [-u] [-n ##] [-l ##] [host] port\n");
				exit(1);
			}
			source = 0;
			break;
		case 's':
			if (source == 0) {
				printf("usage: tsock [-p|-s] [-u] [-n ##] [-l ##] [host] port\n");
				exit(1) ;
			}
			source = 1;
			host_name = argv[argc-2];
			break;
		case 'u':
			protocole = 1;
			break;
		default:
			printf("usage: tsock [-p|-s] [-u] [-n ##] [-l ##] [host] port\n");
			break;
		}
	}
	if (source == -1) {
		printf("usage: tsock [-p|-s] [-u] [-n ##] [-l ##] [host] port\n");
		exit(1) ;
	}
	nb_port = atoi(argv[argc-1]);
	nb_port_htons = htons(nb_port);
	if (nb_port == -1){
		printf("usage: tsock [-p|-s] [-u] [-n ##] [-l ##] port\n");
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
	// RECEPTION - PUIT
	if(source == 0){
		struct sockaddr_in adr_local;
		memset((char *)&adr_local, 0, sizeof(adr_local));
		adr_local.sin_family = AF_INET;
		adr_local.sin_port = nb_port;
		adr_local.sin_addr.s_addr = INADDR_ANY;
		int lg_adr_local = sizeof(adr_local);
		if(bind(sock,(struct sockaddr *)&adr_local, lg_adr_local) == -1){
				perror("Echec du bind\n");
				exit(1);
		}
		char msg[len_message];
		int i = 1;
		int nb_lu;
		// RECEPTION TCP
		if(protocole == 0){
			listen(sock,10);
			int sock_bis;
			printf("PUITS : lg_mesg-lu=%d, port=%d, nb_reception=infini, TP=%s\n",len_message,nb_port,proc);
			if((sock_bis=accept(sock,(struct sockaddr *)&adr_local,&lg_adr_local)) == -1){
					perror("Echec accept");
			}

			while(1){
				if(nb_lu=read(sock_bis,msg,len_message)){
					printf("PUITS : Reception n°%d (%d) ",i,nb_lu);
					afficher_message(msg,nb_lu);
					i++;
				}else{
					if(close(sock_bis) == -1){
						perror("Echec destruction du socket source");
					}else{
						printf("-- Destruction socket --\n");
					}
					if((sock_bis=accept(sock,(struct sockaddr *)&adr_local,&lg_adr_local)) == -1){
						perror("Echec accept");
					}else{
						printf("-- Nouvelle connexion --\n.");
						i=0;
					}
				}
			}
			if(close(sock_bis) == -1){
				perror("destruction du socket source");
			}
			if(shutdown(sock,0) == -1){
				perror("Echec shutdown");
			}
		}else{
			// RECEPTION UDP
			struct sockaddr_in adr_distant;
			memset((char *)&adr_distant, 0, sizeof(adr_distant));
			int size_adr_local = sizeof(adr_local);
			int lg_adr_distant = sizeof(adr_distant);
			printf("PUITS : lg_mesg-lu=%d, port=%d, nb_reception=infini, TP=%s\n",len_message,nb_port,proc);
			while(1){
				if((nb_lu = recvfrom(sock,msg,len_message,0,(struct sockaddr *)&adr_distant,&lg_adr_distant))!=-1){
					printf("PUITS : Reception n°%d (%d) ",i,nb_lu);
					afficher_message(msg,nb_lu);
					i++;
				}
			}
		}
	}else{
		// ENVOI - SOURCE
		struct hostent *hp;
		struct sockaddr_in adr_distant;
		memset((char *)&adr_distant, 0, sizeof(adr_distant));
		adr_distant.sin_family = AF_INET;
		adr_distant.sin_port = nb_port;
		if((hp = gethostbyname(host_name)) == NULL){
				perror("Erreur gethostbyname");
				exit(1);
		}
		memcpy((char*)&(adr_distant.sin_addr.s_addr),
				hp->h_addr,
				hp->h_length);
		int lg_adr_distant = sizeof(adr_distant);
		int nb_char_sent, i;
		if(protocole == 1){
			//ENVOI UDP
			char * msg = malloc(sizeof(char)*len_message);
			printf("SOURCE : port=%d, nb_reception=infini, nb_envois=%d, TP=%s, dest=%s\n",nb_port,nb_message,proc,host_name);
			for(i = 0 ; i < nb_message ; i++){
				construire_message(msg,'a',len_message);
				int ret = sendto(sock,msg,len_message,0,(struct sockaddr*)&adr_distant,sizeof(adr_distant));
				printf("SOURCE : Envoi n°%d (%d) ",(i+1),len_message);
				afficher_message(msg,len_message);
			}
			printf("SOURCE : Fin\n");
		}else{
			//ENVOI TCP
			if(connect(sock,(struct sockaddr *)&adr_distant,lg_adr_distant) != 0){
				perror("Erreur connect");
				exit(1);
			}else{
				char * msg = malloc(sizeof(char)*len_message);
				printf("SOURCE : port=%d, nb_reception=infini, nb_envois=%d, TP=%s, dest=%s\n",nb_port,nb_message,proc,host_name);
				for(i = 0 ; i < nb_message ; i++){
					construire_message(msg,'a',len_message);
					int ret = write(sock,msg,len_message);
					printf("SOURCE : Envoi n°%d (%d) ",(i+1),len_message);
					afficher_message(msg,len_message);
				}
				printf("SOURCE : Fin\n");
			}
		}
	}
	return 0;
}