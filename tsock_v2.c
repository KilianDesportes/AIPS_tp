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
	int soc; /* Socket */
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
		case 'l':
			len_message = atoi(optarg);
			break;
		case 'u':
			protocole = 1;
			break;
		case 'n':
			nb_message = atoi(optarg);
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
		char * msg = malloc(sizeof(char)*len_message);
		int i = 1;
		// RECEPTION TCP
		if(protocole == 0){
			int sock_bis;
			printf("PUITS : lg_mesg-lu=%d, port=%d, nb_reception=infini, TP=%s\n",len_message,nb_port,proc);
			while(1){
				if((sock_bis=accept(sock,(struct sockaddr *)&adr_local,&lg_adr_local)) == -1){
					//ECHEC ACCEPT
				}else{
					if(read(sock_bis,msg,len_message)!=-1){
					printf("PUITS : Reception n°%d (%d) [%s]\n",i,len_message,msg);
					i++;
				}
				}
			}
		}else{
			// RECEPTION UDP
			struct sockaddr_in adr_distant;
			memset((char *)&adr_distant, 0, sizeof(adr_distant));
			int size_adr_local = sizeof(adr_local);
			int lg_adr_distant = sizeof(adr_distant);
			printf("PUITS : lg_mesg-lu=%d, port=%d, nb_reception=infini, TP=%s\n",len_message,nb_port,proc);
			while(1){
				if(recvfrom(sock,msg,len_message,0,(struct sockaddr *)&adr_distant,&lg_adr_distant)!=-1){
					printf("PUITS : Reception n°%d (%d) [%s]\n",i,len_message,msg);
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

		int nb_char_sent, i;
		if(protocole == 1){
			//ENVOI UDP
			printf("SOURCE : port=%d, nb_reception=infini, nb_envois=%d, TP=%s, dest=%s\n",nb_port,nb_message,proc,host_name);
			for(i = 0 ; i < nb_message ; i++){
				char * msg = malloc(sizeof(char)*len_message);
				construire_message(msg,'a',len_message);
				int ret = sendto(sock,msg,len_message,0,(struct sockaddr*)&adr_distant,sizeof(adr_distant));
				printf("SOURCE : Envoi n°%d (%d) [%s]\n",i,len_message,msg);
			}
			printf("SOURCE : Fin\n");
		}else{
			//ENVOI TCP
			if(connect(sock,(struct sockaddr *)&adr_distant,lg_adr_distant) != 0){
				perror("Erreur connect");
				exit(1);
			}else{
				printf("SOURCE : port=%d, nb_reception=infini, nb_envois=%d, TP=%s, dest=%s\n",nb_port,nb_message,proc,host_name);
				for(i = 0 ; i < nb_message ; i++){
					char * msg = malloc(sizeof(char)*len_message);
					construire_message(msg,'a',len_message);
					int ret = write(sock,msg,len_message);
					printf("SOURCE : Envoi n°%d (%d) [%s]\n",i,len_message,msg);
				}
				printf("SOURCE : Fin\n");
			}
		}
	}
	return 0;
}