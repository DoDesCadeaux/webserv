#include "../includes/Server.hpp"

Server::Server()
{
	FD_ZERO(&_allfds);
	FD_ZERO(&_readfds);
	FD_ZERO(&_writefds);
	_listfds.clear();
	// _name = "localhost";
}

Server	&Server::operator=(const Server &other)
{
	_ports = other._ports;
	_name = other._name;
	_root = other._root;
    _errorPage = other._errorPage;
	_locations = other._locations;
	_listfds = other._listfds;
	_allfds = other._allfds;
	_readfds = other._readfds;
	_writefds = other._writefds;
	_maxfd = other._maxfd;
	_clients = other._clients;
	return (*this);
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
		memset(&hint, 0, sizeof(hint));
		hint.ai_family = AF_UNSPEC;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_flags = AI_PASSIVE;

		if (getaddrinfo(NULL, it->first.c_str(), &hint, &servinfo) != 0)
		{
			perror("Address Info");
			exit(EXIT_FAILURE);
		}

		server_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
		if (server_fd == -1)
		{
			perror("Socket");
			exit(EXIT_FAILURE);
		}
		it->second = server_fd;

		fcntl(server_fd, F_SETFL, O_NONBLOCK);

		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		{
			perror("Setsockopt");
			exit(EXIT_FAILURE);
		}

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
///CGI
// add path to request??
///
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

	Ft::printLogs(*this, *_clients[fd], RESPONSE);
	return (n == -1 ? false : true);
}

void Server::run()
{
	int res;

	int count = 0;
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
					std::cout << count << std::endl;
					++count;
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
						
						if (isCGIRequest(_clients[fd]->getRequest())) {
							handleCGIRequest(fd);
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

//////////CGI
bool	Server::isCGIRequest(const Request &request) {
	std::cout << "Uri = " << request.getUri() << std::endl;
	std::cout << "Protocol = " << request.getProtocol() << std::endl;

	if (request.getUri() == CGI_SCRIPT_PATH)
		return (true);
	if (request.getUri() == CGI_UPLOAD_SCRIPT_PATH /*&& request.getProtocol() == "POST"*/) {
std::cout << "---Form html page---\n";
		return (true);
	}

	std::cout << "It's not a CGI request\r\n";

	return (false);
}

bool	Server::handleCGIRequest(int fd) { // --> should I relly return something?
	int 	status = 0;
	pid_t 	pid;

	pid = fork();
	if (pid < 0) {			// Handle error
		return (1);
	}
	else if (pid == 0) {	// Child process
		// Rdirect stdin and stdout
		dup2(fd, 0);
		dup2(fd, 1);

		// set env variables
		//Server::setEnv() ??

		//execute CGI
		//execve() ??
		//protect execve()

	}
	else {					// Parent process
		waitpid(pid, &status, 0);
	}
	return (status); // ??
}
////////////


void Server::newConnection(const int &listen_fd)
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
	_clients[newfd] = new Client(newfd, their_addr, true, listen_fd);

	Ft::printLogs(*this, *_clients[newfd], CONNEXION);
	FD_CLR(listen_fd, &_readfds);
}

void Server::killConnection(const int &fd)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);

	if (it != _clients.end())
	{
		Ft::printLogs(*this, *it->second, DISCONNECT);
		delete it->second;
		_clients.erase(fd);
	}

	close(fd);
	removeFd(fd);
}

std::string Server::getResourceContent(const std::string &uri) {
	std::string fullpath = SERVER_ROOT;

//	std::cout << "uri = " << uri << std::endl;

	if (uri == "/" || uri == "/index")
		fullpath += "/index.html";
	else if (uri == "/favicon.ico")
		fullpath += "/favicon.ico";
//	else if (uri == "/form")
//		fullpath += "/form.html";
	else {
		fullpath += uri;
		if (uri.find('.') == std::string::npos)
			fullpath += ".html";
	}

	if (Ft::fileExists(fullpath)) {
		std::ifstream file(fullpath.c_str(), std::ios::binary);
		return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	}

	return "";
}

std::string &Server::getServerName()
{
	return _name;
}

std::string &Server::getRoot()
{
	return _root;
}

std::map<std::string, int> &Server::getPorts()
{
	return _ports;
}

void Server::saveFile(const int &fd, const std::string &fileData, const std::string &directoryPath, const std::string &mimeType)
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

std::string Server::getMimeType(const std::string &uri) {
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

std::string Server::getExtensionFromMimeType(const std::string &mimeType) {
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

std::vector<Location> &Server::getLocations()
{
	return _locations;
}

void	Server::setServerName(std::string const &name){
	_name = name;
}