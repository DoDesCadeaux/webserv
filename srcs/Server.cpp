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

		if (listen(server_fd, 1000) < 0)
		{
			perror("Listen");
			// exit(EXIT_FAILURE);
			exit(4);
		}
		// Free du addrinfo
		freeaddrinfo(servinfo);
		addFd(server_fd);
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

void Server::removeFd(int fd) {
	FD_CLR(fd, &_allfds);
	for (std::list<int>::iterator it = _listfds.begin(); it != _listfds.end(); ++it) {
		if (*it == fd)
		{
			_listfds.erase(it);
			break;
		}
	}
	if (fd == _maxfd)
		_maxfd = _listfds.back();
}

// Lis une requête entrante sur le socket
// Continuer à appeler recv() jusqu'à ce que tout le contenu soit lu ou qu'il y ait une erreur/fermeture de la connexion
// Analyse ensuite la requête reçue
// Retourne la valeur de l'erreur ou le nombre d'octets lus lors du dernier recv()
int Server::recvAll(const int &fd)
{
	ssize_t bytesRead = BUFFER_SIZE - 1;
	char tmp[BUFFER_SIZE];
	FD_CLR(fd, &_readfds);
	std::string	requestformat;

	// Lire les données entrantes jusqu'à ce qu'il n'y ait plus rien à lire
	while (bytesRead == BUFFER_SIZE - 1)
	{
		// Avec MSG_DONTWAIT recv ne bloquera pas la socket s'il n'y a rien à lire
		bytesRead = recv(fd, tmp, BUFFER_SIZE - 1, MSG_DONTWAIT);
		if (bytesRead > 0)
		{
			tmp[bytesRead] = '\0';
			requestformat += tmp;
		}
		else
			return (-1);
	}

	if (!requestformat.empty())
	{
		Request request(requestformat);
		_clients[fd]->setClientRequest(request);
		if (request.getHeader("Connection") == "keep-alive") {
			_clients[fd]->setKeepAlive(true);
		} else {
			_clients[fd]->setKeepAlive(false);
		}
	}
	return (bytesRead);
}

int Server::sendAll(const int &fd, const std::string &httpResponse, unsigned int *len)
{
	unsigned int total = 0;
	int bytesleft = *len;
	int n;
	int retries = 0;
	FD_CLR(fd, &_writefds);

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
					break;
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

void Server::run() {
	int res;
	struct timeval timeout;

	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	while (true) {
		_readfds = _allfds;
		_writefds = _allfds;

		res = select(_maxfd + 1, &_readfds, &_writefds, NULL, &timeout);
		if (res == -1) {
			std::cout << "error" << std::endl;
		} else {
			std::vector<int> fdsToRemove;

			for (int fd = 0; fd <= _maxfd; ++fd) {
				if (FD_ISSET(fd, &_allfds)) {
					if (FD_ISSET(fd, &_readfds))
						newConnection(fd);

					if (_clients.find(fd) != _clients.end()) {
						if (FD_ISSET(fd, &_readfds)) {
							if (recvAll(fd) == -1) {
								perror("recvAll()");
								std::cout << "echec avec le fd :" << fd << std::endl;
								fdsToRemove.push_back(fd); // Marquez pour suppression
								continue;
							}
							FD_CLR(fd, &_readfds);
						}

						if (FD_ISSET(fd, &_writefds)) {
							std::string uri = _clients[fd]->getRequestUri();
							std::string htmlContent = getResourceContent(fd);
							std::string mimeType = getMimeType(uri);
							std::string httpResponse = HttpResponse::getResponse(200, "OK", htmlContent, mimeType);
							unsigned int len = strlen(httpResponse.c_str());

							std::cout << httpResponse << std::endl;

							if (sendAll(fd, httpResponse, &len) == -1) {
								perror("sendall");
								printf("We only sent %d bytes because of the error!\n", len);
								fdsToRemove.push_back(fd); // Marquez pour suppression
								continue;
							}

							if (!_clients[fd]->isKeepAlive())
								fdsToRemove.push_back(fd);
							else
								_clients[fd]->resetKeepAliveTimer();
						}
					}
				}
			}

			// Supprimez les clients marqués pour suppression
			for (size_t i = 0; i < fdsToRemove.size(); ++i) {
				killConnection(fdsToRemove[i]);
				_clients.erase(fdsToRemove[i]);
			}

			// Gestion des timeouts pour les connexions keep-alive
			for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end();) {
				if (it->second->isKeepAlive() && it->second->hasKeepAliveTimedOut(KEEP_ALIVE_TIMEOUT)) {
					killConnection(it->first);
					_clients.erase(it++); // Supprimez le client et avancez l'itérateur de manière sûre
				} else {
					++it;
				}
			}
		}
	}
}

void Server::newConnection(const int &fd)
{
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	addr_size = sizeof their_addr;

	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (fd == it->second->getFdPort()) {
			std::cout << "Le client existe deja avec port : " << it->first << std::endl;
			return;
		}
	}

	int newfd = accept(fd, (struct sockaddr *)&their_addr, &addr_size);
	if (newfd == -1) {
		return;
	}

	std::cout << GREEN << "New Client connected : " << newfd << NOCOL << std::endl;

	fcntl(newfd, F_SETFL, O_NONBLOCK);
	addFd(newfd);
	_clients[newfd] = new Client(newfd, their_addr, true, fd);
}

void Server::killConnection(const int &fd)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);

	if (it != _clients.end())
	{
		delete it->second; // Supprimer l'objet pointé, si nécessaire
		_clients.erase(fd);  // Supprimer l'entrée de la map
	}
	std::cout << RED << "Client disconnected : " << fd << NOCOL << std::endl;
	close(fd);
	removeFd(fd);
}

std::string Server::getResourceContent(const int &fd) {
	std::string uri = _clients[fd]->getRequestUri();
	std::string fullpath;

	std::cout << RED << "URI : [" << uri << "]" << NOCOL << std::endl;

	if (uri == "/" || uri == "/index") {
		fullpath = "web/index.html";
	} else if (uri == "/favicon.ico") {
		fullpath = "web/favicon.ico";
	} else {
		// Ajouter .html par défaut si aucune extension n'est présente
		if (uri.find('.') == std::string::npos) {
			uri += ".html";
		}
		fullpath = "web" + uri;
	}

	if (!Ft::fileExists(fullpath)) {
		std::ifstream file("web/notFound.html", std::ifstream::binary);
		if (file) {
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			return content;
		}
	} else {
		std::ifstream file(fullpath, std::ios::binary);
		if (file) {
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			return content;
		}
	}
	return "";
}

std::string Server::getMimeType(const std::string& uri) {
	if (Ft::endsWith(uri, ".html")) {
		return "text/html";
	} else if (Ft::endsWith(uri, ".ico")) {
		return "image/x-icon";
	}
	// Ajoutez d'autres types MIME au besoin
	return "text/html";
}


