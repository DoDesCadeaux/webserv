#include "../includes/Server.hpp"

Server::Server()
{
	FD_ZERO(&_allfds);
	FD_ZERO(&_readfds);
	FD_ZERO(&_writefds);
	_listfds.clear();
	_name = "localhost";
}

Server::~Server() {}

void Server::setSocket()
{
	struct addrinfo hint, *servinfo;
	int server_fd;
	int yes = 1;

	std::string port1 = "1918";
	std::string port2 = "8081";
	_ports.insert(std::make_pair(port1, 0));
	_ports.insert(std::make_pair(port2, 0));


	for (std::map<std::string, int>::iterator it = _ports.begin(); it != _ports.end(); it++)
	{
		// On s'assure que la structure est entierement vide
		memset(&hint, 0, sizeof(hint)); // Pas oublier de free quand un truc fail après
		// Parametrage de la structure tampon (hint)
		hint.ai_family = AF_UNSPEC;		// Quelque soit l'ipv
		hint.ai_socktype = SOCK_STREAM; // precise type socket (streaming)
		hint.ai_flags = AI_PASSIVE;		// Assigner localhost au socket

		// Set up les infos du server correctement grace aux params de la struc tampon (hint)
		if (getaddrinfo(NULL, it->first.c_str(), &hint, &servinfo) != 0)
		{
			perror("Address Info");
			exit(EXIT_FAILURE);
		}

		// Mise en place du socket général
		server_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
		if (server_fd == -1)
		{
			perror("Socket");
			exit(EXIT_FAILURE);
		}
		it->second = server_fd;
	

		// Set up socket en non-bloquant
		fcntl(server_fd, F_SETFL, O_NONBLOCK);

		// En cas de re-run du server protection echec bind : "address already in use"
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		{
			perror("Setsockopt");
			exit(EXIT_FAILURE);
		}

		// Attribution du socket au port
		if (bind(server_fd, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
		{
			perror("Bind");
			exit(EXIT_FAILURE);
		}

		if (listen(server_fd, 1000) < 0)
		{
			perror("Listen");
			exit(EXIT_FAILURE);
		}
		// Free du addrinfo
		freeaddrinfo(servinfo);
		addFd(server_fd);
	}

}

void Server::addFd(int fd)
{
	FD_SET(fd, &_allfds);
	_listfds.push_back(fd);
	_listfds.sort();
	_maxfd = _listfds.back();
}

void Server::removeFd(int fd)
{
	FD_CLR(fd, &_allfds);
	for (std::list<int>::iterator it = _listfds.begin(); it != _listfds.end(); ++it)
	{
		if (*it == fd)
		{
			_listfds.erase(it);
			break;
		}
	}
	if (fd == _maxfd)
		_maxfd = _listfds.back();
}

bool Server::recvAll(const int &fd)
{
	std::vector<char> buffer;
	ssize_t bytesRead;
	char tmp[BUFFER_SIZE];

	while (true)
	{
		bytesRead = recv(fd, tmp, BUFFER_SIZE, 0);
		if (bytesRead > 0)
			buffer.insert(buffer.end(), tmp, tmp + bytesRead);
		else
			break;
	}

	if (!buffer.empty())
	{
		Request request(std::string(buffer.begin(), buffer.end()));
		_clients[fd]->setClientRequest(request);
		Ft::printLogs(*this, *_clients[fd], REQUEST);
		if (request.getHeader("Connection") == "keep-alive")
			_clients[fd]->setKeepAlive(true);
		else
			_clients[fd]->setKeepAlive(false);
	}

	return buffer.size();
}

bool Server::sendAll(const int &fd)
{
	std::string uri = _clients[fd]->getRequestUri();
	std::string content = getResourceContent(uri);
	std::string httpResponse;

	if (content.empty())
		httpResponse = HttpResponse::getErrorResponse(404, "Not Found");
	else
	{
		std::string mimeType = getMimeType(uri);
		httpResponse = HttpResponse::getResponse(200, "OK", content, mimeType);
	}

	unsigned int len = httpResponse.length();
	unsigned int total = 0;
	int bytesleft = len;
	int n;
	int retries = 0;
	FD_CLR(fd, &_writefds);

	while (total < len)
	{
		n = send(fd, httpResponse.c_str() + total, bytesleft, 0);
		if (n == -1)
		{
			usleep(20000);
			retries++;
			if (retries > 5)
				break;
			continue;
		}
		total += n;
		bytesleft -= n;
		retries = 0; // Réinitialiser le compteur de réessais après un envoi réussi
	}

	len = total;
	return (n == -1 ? false : true);
}

void Server::run()
{
	int res;

	while (true)
	{
		struct timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		_readfds = _allfds;
		_writefds = _allfds;

		res = select(_maxfd + 1, &_readfds, &_writefds, NULL, &timeout);
		if (res == -1)
		{
			std::cout << "error" << std::endl;
		}
		else
		{
			std::vector<int> fdsToRemove;

			for (int fd = 0; fd <= _maxfd; ++fd)
			{
				if (FD_ISSET(fd, &_readfds))
				{
					usleep(500);
					newConnection(fd);
				}
				if (_clients.find(fd) != _clients.end())
				{
					if (FD_ISSET(fd, &_readfds))
					{
						if (!recvAll(fd))
						{
							fdsToRemove.push_back(fd);
							continue;
						}
					}
					if (FD_ISSET(fd, &_writefds))
					{
						if (FD_ISSET(fd, &_readfds))
						{
							if (_clients[fd]->getRequestProtocol() == "POST")
								saveImage(_clients[fd]->getBodyPayload(), "web/");
						}
						else
						{
							if (!sendAll(fd))
							{
								fdsToRemove.push_back(fd);
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

			// Gestion des timeouts pour les connexions keep-alive
			for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
				if (it->second->isKeepAlive() && it->second->hasKeepAliveTimedOut(KEEP_ALIVE_TIMEOUT))
					fdsToRemove.push_back(it->first);

			// Supprimez les clients marqués pour suppression
			for (size_t i = 0; i < fdsToRemove.size(); ++i)
				killConnection(fdsToRemove[i]);
		}
	}
}

// std::string Server::generateRandomFileName(const std::string& extension) {
//	// Obtenir l'heure actuelle
//	std::time_t currentTime = std::time(NULL);
//
//	// Générer un nombre aléatoire
//	int randomNum = std::rand();
//
//	// Créer un nom de fichier basé sur l'heure et le nombre aléatoire
//	std::string fileName = "image_" + std::to_string(currentTime) + "_" + std::to_string(randomNum) + extension;
//
//	return fileName;
// }

void Server::saveImage(const std::string &imageData, const std::string &directoryPath)
{
	std::string filePath = directoryPath + "image" + (".jpeg");

	std::ofstream fileStream(filePath.c_str(), std::ios::out | std::ios::binary);

	if (!fileStream)
	{
		std::cerr << "Erreur lors de la création du fichier" << std::endl;
		return;
	}

	if (!fileStream.good())
	{
		std::cerr << "Erreur lors de l'écriture dans le fichier" << std::endl;
	}

	fileStream.write(imageData.c_str(), imageData.size());
	fileStream.close();
}

void Server::newConnection(const int &listen_fd)
{
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof their_addr;
	
	// Quid de la vérification de l'adresse aussi
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (listen_fd == it->first){
			return;
		}
	}

	int newfd = accept(listen_fd, (struct sockaddr *)&their_addr, &addr_size);
	if (newfd == -1)
		return;

	fcntl(newfd, F_SETFL, O_NONBLOCK);
	addFd(newfd);
	_clients[newfd] = new Client(newfd, their_addr, true, listen_fd);

	Ft::printLogs(*this, *_clients[newfd], CONNEXION);
	// std::cout << GREEN << "New client : " << newfd << NOCOL << std::endl;
	FD_CLR(listen_fd, &_readfds);
}

void Server::killConnection(const int &fd)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);

	if (it != _clients.end())
	{
		Ft::printLogs(*this, *it->second, DISCONNECT);
		delete it->second;	// Supprimer l'objet pointé, si nécessaire
		_clients.erase(fd); // Supprimer l'entrée de la map
	}

	close(fd);
	removeFd(fd);
}

std::string Server::getResourceContent(const std::string &uri)
{
	std::string fullpath = SERVER_ROOT;

	if (uri == "/" || uri == "/index")
	{
		fullpath += "/index.html";
	}
	else if (uri == "/favicon.ico")
	{
		fullpath += "/favicon.ico";
	}
	else
	{
		fullpath += uri;
		if (uri.find('.') == std::string::npos)
		{
			fullpath += ".html";
		}
	}

	if (Ft::fileExists(fullpath))
	{
		std::ifstream file(fullpath.c_str(), std::ios::binary);
		return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}

	return "";
}

std::string Server::getMimeType(const std::string &uri)
{
	if (Ft::endsWith(uri, ".html"))
		return "text/html";
	else if (Ft::endsWith(uri, ".ico"))
		return "image/x-icon";
	else if (Ft::endsWith(uri, ".jpeg"))
		return "image/jpeg";
	// Ajoutez d'autres types MIME au besoin
	return "text/html";
}

std::string &Server::getServerName(){
	return _name;
}

std::map<std::string, int> &Server::getPorts(){
	return _ports;
}
