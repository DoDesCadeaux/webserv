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
#include "Request.hpp"
#include <list>
#include <algorithm>
#include <map>
#include <sys/socket.h>
#include "Client.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>

# define BUFFER_SIZE 4096 //test values under 4

class Request;
class Client;

class Server
{
	private:
		std::list<char *>				_ports;
		std::list<int>					_listfds;
		fd_set							_sock;
		fd_set							_allfds;
		fd_set							_readfds;
		fd_set							_writefds;
		int 							_maxfd;
		std::string						_buff;
		Request							_req;
		// int								_nbclient = 0;
		std::map<std::string, Client*>			_clients;
		//	std::vector<int>				_clients;
		// container? workers; => List de res de socket()
		// container? client: => List de res de accept()

	public:
		Server();
		~Server();
		void 	setSocket();
		void	addFd(int fd);
		void	run();
		int		sendAll(int fd,  const std::string &httpResponse, unsigned int *len);
		int		recvAll(int fd);
		void	newConnection(int fd);
		// void	accept();
		// kill conmnexion
		// run server avec le select
		// revc
		// send
		// update fd_set
};

std::string addressToString(struct sockaddr_storage &their_addr);

#endif
