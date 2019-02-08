#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

void
construire_message (char *message, char motif, int lg)
{
  int i;
  for (i = 0; i < lg; i++)
    {
      message[i] = motif;
    }
}

void
afficher_message (char *message, int lg)
{
  int i;
  printf ("[");
  for (i = 0; i < lg; i++)
    {
      printf ("%c", message[i]);
    }
  printf ("]\n");
}

int
main (int argc, char **argv)
{
  int c;
  extern char *optarg;
  extern int optind;
  int nb_message = 10;		/* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
  int source = -1;		/* 0 = Serveur, 1 = Client */
  int type_client = -1;		/* 0 = Recepteur , 1 = Emetteur */
  int len_message = 30;		/* Longueur du message, defaut : 30 */
  int nb_msg_modif = 0;		/* 1 = modification, 0 = pas de modification */
  int nb_port = -1;		/* Numéro de port */
  int protocole = 0;		/* 0 = TCP, 1 = UDP */
  char *host_name;		/* Nom d'hôte */
  int sock;			/* Socket */
  char *proc;			/* Nom du protocole */
  // Partie INITIALISATION DES PARAMETRES
  while ((c = getopt (argc, argv, "creusl:n:")) != -1)	/* Fonction pour récuperer les arguments à l'éxécution */
    {
      switch (c)
	{
	case 's':		/* Argument pour dire que c'est le serveur */
	  if (source == 1)
	    {
	      printf
		("usage: tsock [-c [-e|-r]|-s] [-n ##] [-l ##] [host] port\n");
	      exit (1);
	    }
	  source = 0;
	  break;
	case 'c':		/* Argument pour dire que c'est la source */
	  if (source == 0)
	    {
	      printf
		("usage: tsock [-c [-e|-r]|-s] [-n ##] [-l ##] [host] port\n");
	      exit (1);
	    }
	  source = 1;
	  host_name = argv[argc - 2];
	  break;
	case 'r':		/* Argument pour dire que c'est une entité receptrice */
	  type_client = 0;
	  break;
	case 'e':		/* Argument pour dire que c'est une entité receptrice */
	  type_client = 1;
	  break;
	case 'l':		/* Argument pour spécifier la longueur envoyée ou recue */
	  len_message = atoi (optarg);
	  break;
	case 'u':		/* Argument pour spécifier le protocole UDP */
	  protocole = 1;
	  break;
	case 'n':		/* Argument pour spécifier le nombre de message recu */
	  nb_message = atoi (optarg);
	  nb_msg_modif = 1;
	  break;
	default:
	  printf
	    ("usage: tsock [-c [-e|-r]|-s] [-n ##] [-l ##] [host] port\n");
	  break;
	}
    }
  if (source == -1)		/* Si le type de source n'est pas spécifié, on lève une erreur */
    {
      printf ("usage: tsock [-c [-e|-r]|-s] [-n ##] [-l ##] [host] port\n");
      exit (1);
    }

  if (protocole == 1
      && ((source == 1 && type_client == 0)
	  || (source == 0 && type_client == 1)))
    {				/* Si, en UDP, le client est recepteur ou le serveur emetteur, on lève une erreur , car en UDP, 
				   la notion de client et serveur n'est pas représentative, donc le client ne peut être recepteur, et inversement pour le serveur */
      printf ("usage: tsock [-c [-e|-r]|-s] [-n ##] [-l ##] [host] port\n");
      exit (1);
    }

  nb_port = atoi (argv[argc - 1]);	/* On récupère le numéro de port */
  if (nb_port < 5000)		/* Si le nombre de port est inférieur à 5000, on lève une erreur */
    {
      printf ("Le numéro de port doit être supérieur à 5000.\n");
      exit (1);
    }

  // Partie EMISSION DU MESSAGE
  if (protocole == 1)		/* Si le protocole est UDP */
    {
      if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) == -1)	/* On déclare un socket en UDP et on test si il a bien été affecté */
	{
	  perror ("Echec de création du socket\n");
	  exit (1);
	}
      else
	{
	  proc = "UDP";
	}
    }
  else				/* Protocole TCP */
    {
      if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1)	/* On déclare un socket en TCP et on test si il a bien été affecté */
	{
	  perror ("Echec de création du socket\n");
	  exit (1);
	}
      else
	{
	  proc = "TCP";
	}
    }

  // SERVEUR
  if (protocole == 0)		/* Si le protocole est TCP */
    {
      if (source == 0)		/* Si c'est le serveur */
	{
	  struct sockaddr_in adr_local;	/* On déclare l'adresse locale */
	  memset ((char *) &adr_local, 0, sizeof (adr_local));
	  adr_local.sin_family = AF_INET;
	  adr_local.sin_port = nb_port;
	  adr_local.sin_addr.s_addr = INADDR_ANY;
	  int lg_adr_local = sizeof (adr_local);
	  int option = 1;
	  if (setsockopt	/* On utilise setsockopt pour pouvoir utiliser plusieurs fois le socket, ce qui est utile pour ne pas bloquer le 'bind' 
				   dans le cas ou le serveur envoi des données via sock_bis, et non via sock */
	      (sock, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR),
	       (char *) &option, sizeof (option)) < 0)
	    {
	      printf ("setsockopt failed\n");
	      close (sock);
	      exit (1);
	    }
	  if (bind (sock, (struct sockaddr *) &adr_local, lg_adr_local) == -1)	/* On relit l'adresse locale et le socket */
	    {
	      perror ("Echec du bind\n");
	      exit (1);
	    }
	  char msg[len_message];
	  int i = 1;
	  int nb_lu;
	  listen (sock, 10);	/* Permet au socket d'écouter en attendant une connexion */
	  int sock_bis;
	  while (1)		/* Boucle infinie */
	    {
	      if ((sock_bis = accept (sock, (struct sockaddr *) &adr_local, (socklen_t *) & lg_adr_local)) == -1)	/* On crée un socket local pour la machine, qu'on utilisera ensuite */
		{
		  perror ("Echec accept");
		  exit (1);
		}
	      switch (fork ())	/* On créer un fils pour gérer l'envoi ou la reception, sur le socket crée */
		{
		case -1:
		  perror ("Echec fork");
		  exit (1);
		case 0:
		  close (sock);	/* On ferme le socket du père car le fils ne s'en sert pas */
		  if (type_client == 1)	/* Si le serveur est en emission */
		    {
		      printf
			("SOURCE : port=%d, nb_reception=infini, nb_envois=%d, TP=%s\n",
			 nb_port, nb_message, proc);
		      for (i = 0; i < nb_message; i++)	/* On envoi le nombre de message spécifié (par defaut, 10) */
			{
			  construire_message (msg, (i + 97), len_message);	/* On construit le message */
			  write (sock_bis, msg, len_message);	/* On envoi le message, via sock_bis */
			  printf ("SOURCE : Envoi n°%d (%d) ", (i + 1),
				  len_message);
			  afficher_message (msg, len_message);	/* On affiche ce qu'on envoi */
			}
		      printf ("SOURCE : Fin\n");
		    }
		  else		/* Si le serveur est en reception */
		    {
		      printf
			("PUITS : lg_mesg-lu=%d, port=%d, nb_reception=infini, TP=%s socket n°%d\n",
			 len_message, nb_port, proc, sock_bis);
		      if (nb_msg_modif == 1)	/* On test si on a changé ou non le nombre de message à recevoir */
			{
			  int j;
			  for (j = 0; j < nb_message; j++)	/* On boucle sur le nombre de message à lire */
			    {
			      if ((nb_lu = read (sock_bis, msg, len_message)))	/* On recoit un message est on stock sa taille */
				{
				  printf ("PUITS : Reception n°%d (%d) ", i,
					  nb_lu);
				  afficher_message (msg, nb_lu);	/* On affiche le message recu */
				}
			    }
			}
		      else	/* Si le nombre de message n'est pas spciéfié */
			{
			  while ((nb_lu = read (sock_bis, msg, len_message)) > 0)	/* On lit les messages tant qu'il y en a */
			    {
			      printf ("PUITS : Reception n°%d (%d) ", i,
				      nb_lu);
			      afficher_message (msg, nb_lu);	/* On affiche les messages lu */
			      i++;
			    }
			}
		    }
		  if (close (sock_bis) == -1)	/* Fermeture du socket quand on ne l'utilise plus */
		    {
		      perror ("Echec destruction du socket source");
		    }
		  else
		    {
		      printf ("-- Destruction socket %d --\n", sock_bis);

		    }
		  exit (0);
		default:
		  if (close (sock_bis) == -1)	/* On ferme le socket utilisé par le fils */
		    {
		      perror ("Echec destruction du socket du fils");
		    }
		}

	    }
	}
      else if (type_client == 0)	/* Si c'est le client recepteur */
	{
	  struct hostent *hp;	/* Structure hôte */
	  struct sockaddr_in adr_local;	/* Adresse locale */
	  memset ((char *) &adr_local, 0, sizeof (adr_local));
	  adr_local.sin_family = AF_INET;
	  adr_local.sin_port = nb_port;
	  if ((hp = gethostbyname (host_name)) == NULL)	/* On recupère les données de l'hote */
	    {
	      perror ("Erreur gethostbyname");
	      exit (1);
	    }
	  memcpy ((char *) &(adr_local.sin_addr.s_addr),
		  hp->h_addr, hp->h_length);
	  int lg_adr_local = sizeof (adr_local);
	  int nb_lu;
	  int i = 0;
	  char *msg = malloc (sizeof (char) * len_message);
	  if (connect (sock, (struct sockaddr *) &adr_local, lg_adr_local) != 0)	/* On établi la connexion avec le serveur */
	    {
	      perror ("Erreur connect");
	      exit (1);
	    }
	  else
	    {
	      printf
		("PUITS : lg_mesg-lu=%d, port=%d, nb_reception=infini, TP=%s socket n°%d\n",
		 len_message, nb_port, proc, sock);
	      if (nb_msg_modif == 1)	/* Si la longueur a été spécifié */
		{
		  int j;
		  for (j = 0; j < nb_message; j++)	/* On lit le nombre de message voulu */
		    {
		      if ((nb_lu = read (sock, msg, len_message)))	/* On recoit les messages et on les stocks */
			{
			  printf ("PUITS : Reception n°%d (%d) ", (i + 1),
				  nb_lu);
			  afficher_message (msg, nb_lu);	/* On affiche les messages */
			  i++;
			}
		    }
		}
	      else		/* Si la longueur n'a pas été spécifié */
		{
		  while ((nb_lu = read (sock, msg, len_message)) > 0)	/* On recoit tant qu'il y a des messages */
		    {
		      printf ("PUITS : Reception n°%d (%d) ", (i + 1),
			      nb_lu);
		      afficher_message (msg, nb_lu);	/* On affiche les messages */
		      i++;
		    }
		}
	      if (close (sock) == -1)	/* On ferme le socket lorsque l'on en a plus besoin */
		{
		  perror ("Echec destruction du socket source");
		}
	      else
		{
		  printf ("-- Destruction socket %d --\n", sock);
		}
	    }

	}
      else			/* Lorsque c'est le client emetteur */
	{
	  struct hostent *hp;	/* Structure hôte */
	  struct sockaddr_in adr_distant;	/* Structure adresse distante pour envoi */
	  memset ((char *) &adr_distant, 0, sizeof (adr_distant));
	  adr_distant.sin_family = AF_INET;
	  adr_distant.sin_port = nb_port;
	  if ((hp = gethostbyname (host_name)) == NULL)	/* On récupère les informations de l'hôte */
	    {
	      perror ("Erreur gethostbyname");
	      exit (1);
	    }
	  memcpy ((char *) &(adr_distant.sin_addr.s_addr),
		  hp->h_addr, hp->h_length);
	  int lg_adr_distant = sizeof (adr_distant);
	  int i;
	  if (connect (sock, (struct sockaddr *) &adr_distant, lg_adr_distant) != 0)	/* On se connecte à l'hôte distant */
	    {
	      perror ("Erreur connect");
	      exit (1);
	    }
	  else
	    {
	      char *msg = malloc (sizeof (char) * len_message);
	      printf
		("SOURCE : port=%d, nb_reception=infini, nb_envois=%d, TP=%s, dest=%s\n",
		 nb_port, nb_message, proc, host_name);
	      for (i = 0; i < nb_message; i++)	/* On envoi le nombre de message spécifié, par défaut 10 */
		{
		  construire_message (msg, (i + 97), len_message);	/* On construit le message */
		  write (sock, msg, len_message);	/* On envoi le message */
		  printf ("SOURCE : Envoi n°%d (%d) ", (i + 1), len_message);
		  afficher_message (msg, len_message);	/* On affiche ce que l'on a envoyé */
		}
	      printf ("SOURCE : Fin\n");
	    }
	}
    }
  else
    {
      //PROTOCOLE UDP
      if (type_client == 1)	/* Si c'est en emission */
	{
	  struct hostent *hp;	/* Structure de l'hôte */
	  struct sockaddr_in adr_distant;	/* Structure pour l'adresse */
	  memset ((char *) &adr_distant, 0, sizeof (adr_distant));
	  adr_distant.sin_family = AF_INET;
	  adr_distant.sin_port = nb_port;
	  if ((hp = gethostbyname (host_name)) == NULL)	/* On récupère les informations de l'hôte */
	    {
	      perror ("Erreur gethostbyname");
	      exit (1);
	    }
	  memcpy ((char *) &(adr_distant.sin_addr.s_addr),
		  hp->h_addr, hp->h_length);
	  int i;
	  printf ("SOURCE : port=%d, nb_envois=%d, TP=%s, dest=%s\n", nb_port,
		  nb_message, proc, host_name);
	  for (i = 0; i < nb_message; i++)	/* On envoi le nombre de message */
	    {
	      char *msg = malloc (sizeof (char) * len_message);
	      construire_message (msg, (i + 97), len_message);
	      sendto (sock, msg, len_message, 0,
		      (struct sockaddr *) &adr_distant, sizeof (adr_distant));
	      printf ("SOURCE : Envoi n°%d (%d) ", (i + 1), len_message);
	      afficher_message (msg, len_message);	/* On affiche ce qu'on envoi */
	    }
	  printf ("SOURCE : Fin\n");
	}
      else			/* Si c'est en reception */
	{
	  struct sockaddr_in adr_local;	/* Déclaration de l'adresse locale */
	  memset ((char *) &adr_local, 0, sizeof (adr_local));
	  adr_local.sin_family = AF_INET;
	  adr_local.sin_port = nb_port;
	  adr_local.sin_addr.s_addr = INADDR_ANY;
	  int lg_adr_local = sizeof (adr_local);
	  if (bind (sock, (struct sockaddr *) &adr_local, lg_adr_local) == -1)	/* On relit l'adresse au socket */
	    {
	      perror ("Echec du bind\n");
	      exit (1);
	    }
	  char *msg = malloc (sizeof (char) * len_message);	/* On déclare le message qui va servir de buffer */
	  int i = 1;
	  int nb_lu;
	  printf
	    ("PUITS : lg_mesg-lu=%d, port=%d, nb_reception=infini, TP=%s socket n°%d\n",
	     len_message, nb_port, proc, sock);
	  while (1)
	    {
	      if ((nb_lu = recvfrom (sock, msg, len_message, 0, (struct sockaddr *) &adr_local, (socklen_t *) & lg_adr_local)) != -1);	/* On lit les messages tant qu'il y en a */
	      printf ("PUITS : Reception n°%d (%d) ", i, nb_lu);
	      afficher_message (msg, nb_lu);	/* On affiche les messages lu */
	      i++;
	    }
	}
    }
  return 0;
}
