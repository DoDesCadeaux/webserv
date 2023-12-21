#include "../includes/MasterServer.hpp"

// Constructeurs - Destructeur
MasterServer::MasterServer()
{
	FD_ZERO(&_allfds);
	FD_ZERO(&_readfds);
	FD_ZERO(&_writefds);
	_listfds.clear();
}

MasterServer::~MasterServer()
{
}

MasterServer &MasterServer::operator=(MasterServer const &other)
{
	_ports = other._ports;
	_listfds = other._listfds;
	_allfds = other._allfds;
	_readfds = other._readfds;
	_writefds = other._writefds;
	_maxfd = other._maxfd;
	_clients = other._clients;
	return (*this);
}

// Setter
void MasterServer::setServer(std::vector<Server> &servers)
{
	_servers = servers;
}

// Getter private var
std::map<std::string, int> &MasterServer::getPorts()
{
	return _ports;
}
std::vector<Server> &MasterServer::getServers()
{
	return _servers;
}
std::map<int, Client *> &MasterServer::getClients()
{
	return _clients;
}

// General getter
Server &MasterServer::getServerBySocketPort(int port)
{
	for (std::vector<Server>::iterator it = getServers().begin(); it != getServers().end(); ++it)
	{
		for (std::map<std::string, int>::const_iterator itport = it->getPorts().begin(); itport != it->getPorts().end(); itport++)
		{
			if (itport->second == port)
				return *it;
		}
	}
	throw std::runtime_error("Server not found");
}
Server &MasterServer::getServerByClientSocket(int fd)
{
	for (std::vector<Server>::iterator it = getServers().begin(); it != getServers().end(); ++it)
	{
		for (std::vector<int>::iterator itClient = it->getClients().begin(); itClient != it->getClients().end(); itClient++)
		{
			if (*itClient == fd)
				return *it;
		}
	}
	throw std::runtime_error("Server not found");
}

//Statics Utils
static std::string getResourceContent(const std::string &uri)
{
	std::string fullpath = SERVER_ROOT;

	if (uri == "/" || uri == "/index")
		fullpath += "/index.html";
	else if (uri == "/favicon.ico")
		fullpath += "/favicon.ico";
	else
	{
		fullpath += uri;
		if (uri.find('.') == std::string::npos)
			fullpath += ".html";
	}

	if (Ft::fileExists(fullpath))
	{
		std::ifstream file(fullpath.c_str(), std::ios::binary);
		return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}

	return "";
}

static std::string getExtensionFromMimeType(const std::string &mimeType)
{
	if (mimeType.find("image/jpeg") != std::string::npos)
		return ".jpeg";
	else if (mimeType.find("image/png") != std::string::npos)
		return ".png";
	else if (mimeType.find("image/gif") != std::string::npos)
		return ".gif";
	else if (mimeType.find("text/plain") != std::string::npos)
		return ".txt";
	else if (mimeType.find("text/html") != std::string::npos)
		return ".html";
	else
		return ".bin";
}

static std::string getMimeType(const std::string &uri) {
	if (Ft::endsWith(uri, ".html"))
		return "text/html";
	else if (Ft::endsWith(uri, ".ico"))
		return "image/x-icon";
	else if (Ft::endsWith(uri, ".jpeg"))
		return "image/jpeg";
	else if (Ft::endsWith(uri, ".jpg"))
		return "image/jpg";
	else if (Ft::endsWith(uri, ".gif"))
		return "image/gif";
	return "text/html";
}

// static std::string generateRandomFileName(const std::string& extension)
// {

// }


// Methods
void MasterServer::run()
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
			std::cout << "error" << std::endl;
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
								saveFile(fd, _clients[fd]->getBodyPayload(), "web/", _clients[fd]->getHeaderTypeValue("Content-Type"));
						}
						else if (FD_ISSET(fd, &_writefds))
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

bool MasterServer::recvAll(const int &fd)
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
		Ft::printLogs(getServerByClientSocket(fd), *_clients[fd], REQUEST);
		if (request.getHeader("Connection") == "keep-alive")
			_clients[fd]->setKeepAlive(true);
		else
			_clients[fd]->setKeepAlive(false);
	}

	return buffer.size();
}

bool MasterServer::sendAll(const int &fd)
{
	std::string content;
	std::string uri = _clients[fd]->getRequestUri();
	HttpResponse response;

	if (_clients[fd]->getRequestProtocol() == "GET")
		content = getResourceContent(uri);
	else if (_clients[fd]->getRequestProtocol() == "POST") {
		std::ifstream file(_clients[fd]->getLastFilePath());
		std::string line;
		if (file.is_open()) {
			while (getline(file, line)) {
				content += line + "\r\n";
			}
			file.close();
		}
		else
			std::cerr << "Impossible d'ouvrir le fichier" << std::endl;
	}

	if (content.empty()){
		response.setErrorResponse(404, "Not Found");
		_clients[fd]->setClientResponse(response);
	}
	else
	{
		std::string mimeType = getMimeType(uri);
		if (_clients[fd]->getRequestProtocol() == "GET")
			response.setNormalResponse(200, "OK", content, mimeType, _clients[fd]->getLastFilePath());
		else if (_clients[fd]->getRequestProtocol() == "POST")
			response.setNormalResponse(302, "Found", content, mimeType, _clients[fd]->getLastFilePath());
		_clients[fd]->setClientResponse(response);
	}

	//Faire la fonction de factorisation sur le HttpResponse reponse -> Faire l'operateur d'assignement (HttpResponse & operator=(const  HttpResponse &other))

	unsigned int len = response.getResponse().length();
	unsigned int total = 0;
	int bytesleft = len;
	int n;
	int retries = 0;
	FD_CLR(fd, &_writefds);

	while (total < len)
	{
		n = send(fd, response.getResponse().c_str() + total, bytesleft, 0);
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
		retries = 0;
	}

	Ft::printLogs(getServerByClientSocket(fd), *_clients[fd], RESPONSE);
	return (n == -1 ? false : true);
}

void MasterServer::saveFile(const int &fd, const std::string &fileData, const std::string &directoryPath, const std::string &mimeType)
{
	std::string extension = getExtensionFromMimeType(mimeType);
	std::string filePath = directoryPath + "postedFile" + extension;

	_clients[fd]->setLastFilePath(filePath);

	std::ofstream fileStream(filePath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

	if (!fileStream)
	{
		std::cerr << "Erreur lors de la création du fichier" << std::endl;
		return;
	}

	fileStream.write(fileData.c_str(), fileData.size());

	if (!fileStream.good())
		std::cerr << "Erreur lors de l'écriture dans le fichier" << std::endl;

	fileStream.close();
}

// Methods Utils
void MasterServer::addFd(int fd)
{
	FD_SET(fd, &_allfds);
	_listfds.push_back(fd);
	_listfds.sort();
	_maxfd = _listfds.back();
}

void MasterServer::newConnection(const int &listen_fd)
{
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof their_addr;

	// Quid de la vérification de l'adresse aussi
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (listen_fd == it->first)
			return;
	}

	int newfd = accept(listen_fd, (struct sockaddr *)&their_addr, &addr_size);
	if (newfd == -1)
		return;

	fcntl(newfd, F_SETFL, O_NONBLOCK);
	addFd(newfd);

	Server &server = getServerBySocketPort(listen_fd);

	_clients[newfd] = new Client(newfd, their_addr, true, listen_fd);
	server.addClient(newfd);

	Ft::printLogs(server, *_clients[newfd], CONNEXION);
	FD_CLR(listen_fd, &_readfds);
}

void MasterServer::removeFd(int fd)
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

void MasterServer::killConnection(const int &fd)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);

	if (it != _clients.end())
	{
		Ft::printLogs(getServerByClientSocket(fd), *it->second, DISCONNECT);
		delete it->second;
		_clients.erase(fd);
	}

	close(fd);
	removeFd(fd);
}

