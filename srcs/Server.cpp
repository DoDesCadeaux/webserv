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

#include "../includes/Server.hpp"

Server::Server()
{
	FD_ZERO(&_allfds);
}

Server::~Server() {}

void Server::setSocket()
{
	struct addrinfo hint, *servinfo;
	int server_fd;
	int yes = 1;

	char port1[] = "1918";
	char port2[] = "8081";
	_ports.push_back(port1);
	_ports.push_back(port2);

	for (std::list<char *>::iterator it = _ports.begin(); it != _ports.end(); it++)
	{
		// On s'assure que la structure est entierement vide
		memset(&hint, 0, sizeof(hint)); // Pas oublier de free quand un truc fail après
		// Parametrage de la structure tampon (hint)
		hint.ai_family = AF_UNSPEC;		// Quelque soit l'ipv
		hint.ai_socktype = SOCK_STREAM; // precise type socket (streaming)
		hint.ai_flags = AI_PASSIVE;		// Assigner localhost au socket

		// Set up les infos du server correctement grace aux params de la struc tampon (hint)
		if (getaddrinfo(NULL, *it, &hint, &servinfo) != 0)
		{
			perror("Address Info");
			// exit(EXIT_FAILURE);
			exit(1);
		}

		// Mise en place du socket général
		server_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
		if (server_fd == -1)
		{
			perror("Socket");
			// exit(EXIT_FAILURE);
			exit(2);
		}

		// Set up socket en non-bloquant
		fcntl(server_fd, F_SETFL, O_NONBLOCK);

		// En cas de re-run du server protection echec bind : "address already in use"
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		{
			perror("Setsockopt");
			exit(1);
		}

		// Attribution du socket au port
		if (bind(server_fd, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
		{
			std::cout << *it << ": is used port" << std::endl;
			perror("Bind");
			// exit(EXIT_FAILURE);
			exit(3);
		}

		if (listen(server_fd, 2) < 0)
		{
			perror("Listen");
			// exit(EXIT_FAILURE);
			exit(4);
		}

		// Free du addrinfo
		freeaddrinfo(servinfo);
		addFd(server_fd);
		printf("connexion au port %s avec les socket %d\n", *it, server_fd);
	}
	// free le memset
}

void Server::addFd(int fd)
{
	FD_SET(fd, &_allfds);
	_listfds.push_back(fd);
	_listfds.sort();
	_maxfd = _listfds.back();
}
// Lis une requête entrante sur le socket
// Continuer à appeler recv() jusqu'à ce que tout le contenu soit lu ou qu'il y ait une erreur/fermeture de la connexion
// Analyse ensuite la requête reçue
// Retourne la valeur de l'erreur ou le nombre d'octets lus lors du dernier recv()
int Server::recvAll(int fd)
{
	std::cout << "recv avec le fd :" << fd << std::endl;
	ssize_t bytesRead = BUFFER_SIZE - 1;
	char tmp[BUFFER_SIZE];
	FD_CLR(fd, &_writefds);

	_buff.clear();
	// Lire les données entrantes jusqu'à ce qu'il n'y ait plus rien à lire
	while (bytesRead == BUFFER_SIZE - 1)
	{
		// Avec MSG_DONTWAIT recv ne bloquera pas la socket s'il n'y a rien à lire
		bytesRead = recv(fd, tmp, BUFFER_SIZE - 1, MSG_DONTWAIT);
		if (bytesRead > 0)
		{
			tmp[bytesRead] = '\0';
			_buff += tmp;
		}
		else if (bytesRead == 0)
		{
			std::cout << "Connection was closed" << std::endl;
			return 0;
		}
		else
			return (-1);
	}
	if (_buff.size() > 0)
	{
		Request req(_buff);
		_req = req;
	}
	return (bytesRead);
}

int Server::sendAll(int fd, const std::string &httpResponse, unsigned int *len)
{
	std::cout << "send avec le fd: " << fd << std::endl;
	unsigned int total = 0;
	int bytesleft = *len;
	int n;
	int retries = 0;
	FD_CLR(fd, &_readfds);

	while (total < *len)
	{
		n = send(fd, httpResponse.c_str() + total, bytesleft, 0);
		if (n == -1)
		{
			std::cout << errno << std::endl;
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Le buffer est plein, attendre un peu avant de réessayer
				usleep(20000);
				retries++;
				if (retries > 5)
				{ // Limiter le nombre de réessais pour éviter une boucle infinie
					break;
				}
				continue;
			}
		}
		total += n;
		bytesleft -= n;
		retries = 0; // Réinitialiser le compteur de réessais après un envoi réussi
	}

	*len = total;
	return (n == -1 ? -1 : 0);
}

void Server::run()
{
	struct timeval time;
	// struct sockaddr_storage their_addr;
	// socklen_t addr_size;
	int res;

	_readfds = _allfds;
	_writefds = _allfds;
	time.tv_sec = 1;
	time.tv_usec = 500000;

	res = select(_maxfd + 1, &_readfds, &_writefds, NULL, &time);

	// Ft::printSet(_allfds, "_allfds");
	// Ft::printSet(_readfds, "_readfds");
	// Ft::printSet(_writefds, "_writefds");

	if (res == 0)
		std::cout << "timeout" << std::endl;
	if (res == -1)
		std::cout << "error" << std::endl;
	else
	{
		for (int fd = 0; fd <= _maxfd; ++fd)
		{
			// Le fd est-il lié à notre programme?
			if (FD_ISSET(fd, &_allfds))
			{
				// Un nouveau client veut créer une connexion
				if (FD_ISSET(fd, &_readfds))
					newConnection(fd);
				
				for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
				{
					if (it->second->getFd() == fd)
					{
						std::cout << "CE PUTAIN DE FD " << fd << " APPARTIENT A UN CLIENT" << std::endl;
						// Un client existent nous fait une requête (GET)
						Ft::printSet(_readfds, "read");
						Ft::printSet(_writefds, "write");
						if (FD_ISSET(fd, &_writefds))
						{
							std::cout << "WRITE" << std::endl;

							// Ouvrir le fichier index.html
							std::ifstream htmlFile("index.html");
							// Envoyer le contenu du fichier index.html dans cette variable
							std::string htmlContent((std::istreambuf_iterator<char>(htmlFile)), std::istreambuf_iterator<char>());
							// Ecrire la reponse code HTTP étant OK (200) avec le type de contenu (type html utf-8) et la taille
							std::string httpResponse = "HTTP/1.1 200 OK\r\n"
													   "Content-Type: text/html\r\n"
													   "Content-Length: " +
													   std::to_string(htmlContent.size()) + "\r\n"
																							"\r\n" +
													   htmlContent;
							unsigned int len = strlen(httpResponse.c_str());
							if (sendAll(fd, httpResponse, &len) == -1)
							{
								perror("sendall");
								printf("We only sent %d bytes because of the error!\n", len);
								// exit(EXIT_FAILURE);
								// exit(5);
							}
							htmlFile.close();
							// Interdit il me semble
							std::cout << errno << std::endl;
						}
						// Un client existent nous envoie une requête (PUSH)
						else if (FD_ISSET(fd, &_readfds))
						{
							std::cout << "READ" << std::endl;

							if (recvAll(fd) == -1)
							{
								perror("recvAll()");
								std::cout << "echec avec le fd :" << fd << std::endl;
								// exit(EXIT_FAILURE);
								// exit(6);
							}
							printf("en attente d'ecriture\n");
						}
					}
				}
			}
		}
	}
	// TO DO :
	// if a recv/send error occured,
	// we need to close the active connection,
	// remove it from the master_set and decrease the value of
	// max socket in the master set
}

void Server::newConnection(int fd)
{
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	addr_size = sizeof their_addr;

	FD_CLR(fd, &_readfds);

	int newfd = accept(fd, (struct sockaddr *)&their_addr, &addr_size);
	if (newfd == -1)
		return;
	std::string addr = addressToString(their_addr);
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (fd == it->second->getFdPort())
		{
			close(newfd);
			return;
		}
	}
	fcntl(newfd, F_SETFL, O_NONBLOCK);
	printf("new connection accepted with sock : %d avec fd de base : %d\n", newfd, fd);
	addFd(newfd);
	_clients[newfd] = new Client(newfd, their_addr, true, fd);
}

std::string addressToString(struct sockaddr_storage &their_addr)
{
	char addrstr[INET6_ADDRSTRLEN]; // Assez grand pour IPv6

	if (their_addr.ss_family == AF_INET)
	{
		// C'est une adresse IPv4
		struct sockaddr_in *addr_in = (struct sockaddr_in *)&their_addr;
		inet_ntop(AF_INET, &(addr_in->sin_addr), addrstr, sizeof(addrstr));
	}
	else if (their_addr.ss_family == AF_INET6)
	{
		// C'est une adresse IPv6
		struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&their_addr;
		inet_ntop(AF_INET6, &(addr_in6->sin6_addr), addrstr, sizeof(addrstr));
	}
	else
	{
		// Type d'adresse inconnu
		strcpy(addrstr, "Inconnu");
	}

	return std::string(addrstr);
}