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
#include <list>
#include <algorithm>

#define PORT "7812"

class Server
{
private:
	std::list<char *>	_ports;
	std::list<int>		_listfds;
	fd_set				_allfds;
	fd_set				_readfds;
	fd_set				_writefds;	
	int 				_maxfd;
	// container? workers; => List de res de socket()
	// container? client: => List de res de accept()

public:
	Server();
	~Server();
	void 	setSocket();
	void	addFd(int fd);
	void	run();
	// void	accept();
	// kill conmnexion
	// run server avec le select
	// revc
	// send
	// update fd_set
};

#endif