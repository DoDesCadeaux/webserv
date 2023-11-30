/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamartin <pamartin@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/30 13:03:41 by pamartin          #+#    #+#             */
/*   Updated: 2023/11/30 13:03:42 by pamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include "Parsor.hpp"

class Server
{
private:
	int _sockFD;
	struct addrinfo *_servinfo;

public:
	void setSocket();
	// listen
	// accept connexion
	// kill conmnexion
	// run server avec le select
};

void Server::setSocket()
{
	int status;
	struct addrinfo hints, *rp;

	// Pour etre sur que hint est bien vide
	memset(&hints, 0, sizeof hints);
	// Famille d'addressage renvoyée -> gere ipv4 et 6
	hints.ai_family = AF_UNSPEC;
	// Type de socket
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL, "4242", &hints, &_servinfo);

	if (status != 0)
	{
		printf("getaddrinfo: %s", gai_strerror(status));
	}

	_sockFD = socket(_servinfo->ai_family, _servinfo->ai_socktype, _servinfo->ai_protocol);
	if (_sockFD == -1)
		printf("echec socket");
	fcntl(_sockFD, F_SETFL, O_NONBLOCK);
	if (bind(_sockFD, _servinfo->ai_addr, _servinfo->ai_addrlen) == 0)
	{
		printf("Bind on port 4242 with %d", _sockFD);
	}
	if (_servinfo == NULL)
	{
		printf("could not bind()");
	}
	// close(_sockFD);
	// freeaddrinfo(_servinfo);
}

#endif