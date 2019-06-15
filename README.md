Projet 'Applications internet et programmation socket', 3e année INSA Toulouse, Pré-orientation MIC.

DELIOT Maxence
DESPORTES Kilian

3 MIC
Groupe C
2018-2019.

Programme de communication entre deux instances (source & puit), en TCP ou en UDP.

Nous avons implémenté l'ensemble des fonctionnalités demandés, toutes disponibles dans la version 4.

Génération de l'exécutable : `$ gcc tsock_v4.c -o tsock`

Utilisation :
* Mode puit : `tsock -p [-options] port`
* Mode source : `tsock -s [-options] host port`

Options :
* Utilisation UDP :  `-u`, par defaut, TCP est utilisé
* Longueur (en octets) de messages à émettre (emission) : `-l ##`, par défaut, la longueur est de 30.
* Nombre (en octets) de messages à émettre (emission) : `-n ##`, par défaut, 10 messages sont envoyés.
