/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamartin <pamartin@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/30 13:03:54 by pamartin          #+#    #+#             */
/*   Updated: 2023/11/30 13:03:55 by pamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(){
    FD_ZERO(&_allfds);
}
Server::~Server(){}

void Server::setSocket()
{
	struct addrinfo hint, *servinfo;
	int				server_fd;
	int 			yes = 1;

     char port1[] = "1918";
     char port2[] = "8081";
    _ports.push_back(port1);
    _ports.push_back(port2);

	//ICI RAJOUTER : Boucle pour les worker
	for (std::list<char *>::iterator it=_ports.begin(); it != _ports.end(); it++){
		//On s'assure que la structure est entierement vide
		memset(&hint, 0, sizeof(hint));
		//Parametrage de la structure tampon (hint)
		hint.ai_family = AF_UNSPEC;				//Quelque soit l'ipv
		hint.ai_socktype = SOCK_STREAM;			//precise type socket (streaming)
		hint.ai_flags = AI_PASSIVE;				//Assigner localhost au socket

		//Set up les infos du server correctement grace aux params de la struc tampon (hint)
		if (getaddrinfo(NULL, *it, &hint, &servinfo) != 0) {
			perror("Address Info");
			exit(EXIT_FAILURE);
		}

		//Mise en place du socket général
		server_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol) ;
		if (server_fd == -1) {
			perror("Socket");
			exit(EXIT_FAILURE);
		}

		//Set up socket en non-bloquant
		fcntl(server_fd, F_SETFL, O_NONBLOCK);

		//En cas de re-run du server protection echec bind : "address already in use"
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes) ) == -1) {
			perror("Setsockopt");
			exit(1);
		}

		//Connection du socket au port
		if (bind(server_fd, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
			std::cout << *it << ": is used port" << std::endl;
			perror("Bind");
			exit(EXIT_FAILURE);
		}

		//Free du addrinfo
		freeaddrinfo(servinfo);
        addFd(server_fd);
        printf("connexion au port %s avec les socket %d\n", *it, server_fd);
	}

	//QUID DU close(_sockFD);
}

void Server::addFd(int fd){
    FD_SET(fd, &_allfds);
    _listfds.push_back(fd);
    _listfds.sort();
    _maxfd = _listfds.back();
}





