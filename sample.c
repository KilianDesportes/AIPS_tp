
      printf("PUITS : lg_mesg_lu=%d, port=%d, ", msg_size, port);

      if(msg_nbr == MSG_NBR_INF)
	printf("nb_reception=infini, TP=%s\n", (type == TYPE_UDP) ? "udp" : "tcp");
      else
	printf("nb_reception=%d, TP=%s\n", msg_nbr, (type == TYPE_UDP) ? "udp" : "tcp");

      if ((sfd = socket(domain, type, 0)) == -1)
	ABORT_ERR("Creation du socket serveur (puits)");

      memset(&saddr, 0, sizeof(saddr));

      saddr.sin_family		= domain;
      saddr.sin_port		= htons(port);
      saddr.sin_addr.s_addr	= htonl(INADDR_ANY);

      if (bind(sfd, (struct sockaddr *)&saddr, (socklen_t)lsaddr) == -1)
	ABORT_ERR("echec du bind");

      /* TCP */
      if (type == TYPE_TCP) {

	listen(sfd, DEFAULT_BACK_LOG);

	if ((cfd = accept(sfd, (struct sockaddr*)&caddr, (socklen_t *)&lcaddr)) == -1)
	  ABORT_ERR("accept");

	if(msg_nbr != MSG_NBR_INF)
	  for(i = 0;
	      i < msg_nbr && read(cfd, msg, msg_size);
	      i++) {

	    printf("PUITS : Reception [%5d] (%5d) [%s]\n", i, msg_size, msg);
	  
	  }
	/* The number of message we are waiting for is finite */
	else {
	  i = 0;
	  while (1) {
	    if(read(cfd, msg, msg_size)) {
	      printf("PUITS : Reception [%5u] (%5d) [%s]\n", (unsigned int)i, msg_size, msg);
	      i++;
	    }
	    /* We wait for another connection */
	    else {

	      if (close(cfd) == -1)
		ABORT_ERR("destruction du socket client (source)");
	      if ((cfd = accept(sfd, (struct sockaddr*)&caddr, (socklen_t *)&lcaddr)) == -1)
		ABORT_ERR("accept");
	      
	      printf("============== NOUVELLE CONNEXION ==============\n");
	      
	      i = 0;
	    }
	  }
	}

	if (close(cfd) == -1)
	  ABORT_ERR("destruction du socket client (source)");

	if(shutdown(sfd, 0) == -1)
	  ABORT_ERR("Could not shutdown socket");
