## Projet 'Applications internet et programmation socket' &mdash; INSA Toulouse

DELIOT Maxence

DESPORTES Kilian


3 MIC
Group C
2018-2019.

Program for communication between two instances (source & sink), in TCP or UDP.

We have implemented all the requested functionalities, all available in version 4.

Generating the executable: `$ gcc tsock_v4.c -o tsock`

Use :
* Well mode: `tsock -p [-options] port`
* Source mode: `tsock -s [-options] host port`.

Options :
* UDP usage: `-u`, by default, TCP is used
* Length (in bytes) of messages to be sent (transmission): `-l ##`, by default, the length is 30.
* Number (in bytes) of messages to be sent: `-n ##`, by default, 10 messages are sent.
