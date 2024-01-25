#include "../includes/MasterServer.hpp"

MasterServer* MasterServer::_masterServerPtr = NULL;
// Constructeurs - Destructeur
MasterServer::MasterServer()
{
	FD_ZERO(&_allfds);
	FD_ZERO(&_readfds);
	FD_ZERO(&_writefds);
	_listfds.clear();

}

MasterServer::~MasterServer() {}

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

void MasterServer::initializeMasterServer(MasterServer* masterServerPtr)
{
	_masterServerPtr = masterServerPtr;
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
			{
				return *it;
			}
		}
	}
	throw std::runtime_error("Server not found");
}

static bool isDirectory(const std::string &path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

static std::vector<std::string> getDirectoryContents(const std::string &directoryPath)
{
	std::vector<std::string> contents;
	DIR *dir = opendir(directoryPath.c_str());

	if (dir == NULL)
	{
		return contents;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
			contents.push_back(entry->d_name);
		}
	}

	closedir(dir);
	return contents;
}

static std::string generateDirectoryListing(std::string directoryPath)
{
	std::vector<std::string> contents = getDirectoryContents(directoryPath);
	std::string html = "<!DOCTYPE html><html><head><title>Index</title></head><body><h1>Index of " + directoryPath;
	directoryPath = Ft::startsWith(directoryPath, "./") ? directoryPath.substr(1, std::string::npos) : directoryPath;

	for (std::vector<std::string>::const_iterator it = contents.begin(); it != contents.end(); ++it)
		html += "<li><a href=" + directoryPath + "/" + *it + ">" + *it + "</a></li><hr> </ul></body></html>";

	return html;
}

// Statics Utils
std::string MasterServer::getResourceContent(const std::string &uri, int fd)
{
	std::string tmp = Ft::startsWith(uri, "./") ? uri : (Ft::startsWith(uri, "/") ? "." + uri : "./" + uri);
	std::string fullpath = getServerByClientSocket(fd).getRoot();

	Server server = getServerByClientSocket(fd);
	for (std::vector<Location>::iterator it = server.getLocations().begin(); it != server.getLocations().end(); it++)
	{
		if (!it->autoindex.empty() && it->autoindex == "on")
			if ((it->path == uri || Ft::startsWith(uri, it->path)) && isDirectory(tmp))
				return generateDirectoryListing(tmp);
		if (it->path == uri)
		{
			if (!it->root.empty())
				fullpath = it->root;
			if (!it->index.empty())
				tmp = fullpath + "/" + it->index;
		}
		else if (uri.find(it->path + '?') != std::string::npos)
		{
			if (!it->cgi.empty())
			{
				if (!it->root.empty())
					fullpath = it->root;
				std::map<std::string, std::string>::iterator script = it->cgi.begin();
				handleCGIRequest(*_clients[fd], fullpath + "/" + script->second);
				return (_clients[fd]->getResponseBody());
			}
		}
	}
	if (Ft::fileExists(tmp))
		fullpath = tmp;
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
	else if (mimeType.find("image/jpg") != std::string::npos)
		return ".jpg";
	else if (mimeType.find("image/gif") != std::string::npos)
		return ".gif";
	else if (mimeType.find("text/plain") != std::string::npos)
		return ".txt";
	else if (mimeType.find("text/html") != std::string::npos)
		return ".html";
	else
		return ".bin";
}

static std::string getMimeType(const std::string &uri)
{
	if (Ft::endsWith(uri, ".html"))
		return "text/html";
	else if (Ft::endsWith(uri, ".ico"))
		return "image/x-icon";
	else if (Ft::endsWith(uri, ".jpeg"))
		return "image/jpeg";
	else if (Ft::endsWith(uri, ".jpg"))
		return "image/jpg";
	else if (Ft::endsWith(uri, ".png"))
		return "image/jpg";
	else if (Ft::endsWith(uri, ".gif"))
		return "image/gif";
	else if (Ft::endsWith(uri, ".txt"))
		return "text/plain";
	return "text/html";
}

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
			continue;
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
					if (!sendAll(fd)) {
						HttpResponse response;
						response.setErrorResponse(500, "Internal Server Error");
						_clients[fd]->setClientResponse(response);
						continue;
					}
					if (_clients[fd]->getRequestFormat().empty())
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

		// Gestion des timeouts pour les connexions keep-alive
		for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
			if (it->second->isKeepAlive() && it->second->hasKeepAliveTimedOut(KEEP_ALIVE_TIMEOUT))
				fdsToRemove.push_back(it->first);

		// Supprimez les clients marqués pour suppression
		for (size_t i = 0; i < fdsToRemove.size(); ++i)
			killConnection(fdsToRemove[i]);
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
		if (bytesRead == -1) {
			HttpResponse response;
			response.setErrorResponse(500, "Internal Server Error");
			break;
		}
		if (bytesRead == 0)
			break;
		buffer.insert(buffer.end(), tmp, tmp + bytesRead);
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

static bool bodySizeIsValid(Server server, std::string uri, std::string filePath)
{
	unsigned int maxBodySize;
	std::streampos size = 0;

	std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		size = file.tellg();
		file.close();
	}
	else
		std::cout << "Impossible d'ouvrir le fichier." << std::endl;

	for (std::vector<Location>::iterator it = server.getLocations().begin(); it != server.getLocations().end(); it++)
	{
		if (it->path == uri)
		{
			if (!it->max_body.empty())
				maxBodySize = atol(it->max_body.c_str());
			if (static_cast<unsigned int>(size) > maxBodySize)
				return false;
		}
	}
	return true;
}

static bool send(const int &fd, const char *buf, size_t len)
{
    size_t total = 0;
    ssize_t n;
	int i = 0;

	while (total < len)
	{
		n = send(fd, buf + total, len - total, 0);
		if (n == -1)
			break;
		if (n == 0) {
			usleep(5000);
			i++;
			if (i < 5)
				continue;
			break;
		}
		total += n;
		i = 0;
	}

    return total == len;
}

bool MasterServer::sendAll(const int &fd)
{
	std::string 	content = "";
	std::string 	uri = _clients[fd]->getRequestUri();
	HttpResponse 	response;

	Server server = getServerByClientSocket(fd);
	if (!server.isAuthorizedProtocol(uri, _clients[fd]->getRequestProtocol()))
		response.setErrorResponse(405, "Method Not Allowed");
	else if (_clients[fd]->getRequestProtocol() != "GET" && _clients[fd]->getRequestProtocol() != "POST" && _clients[fd]->getRequestProtocol() != "DELETE")
		response.setErrorResponse(501, "Not Implemented");
	else
	{
		if (_clients[fd]->getRequestProtocol() == "GET") {
			content = getResourceContent(uri, fd);
		}
		else if (_clients[fd]->getRequestProtocol() == "POST")
		{
			std::string tmp = Ft::startsWith(uri, "./") ? uri : (Ft::startsWith(uri, "/") ? "." + uri : "./" + uri);
			std::string fullpath = server.getRoot();

			for (std::vector<Location>::iterator it = server.getLocations().begin(); it != server.getLocations().end(); it++)
			{
				if (it->path == uri)
				{
					if (!it->root.empty())
						fullpath = it->root;
					if (!it->index.empty())
						tmp = fullpath + "/" + it->index;
				}
				else if (uri.find(it->path + '/') != std::string::npos)
				{

					if (!it->cgi.empty())
					{
						if (!it->root.empty())
							fullpath = it->root;
						std::map<std::string, std::string>::iterator script = it->cgi.begin();
						handleCGIRequest(*_clients[fd], fullpath + "/" + script->second);
						saveFile(fd, _clients[fd]->getResponseBody(), "text/html");
						break;
					}
				}
				else
					saveFile(fd, _clients[fd]->getBodyPayload(), _clients[fd]->getHeaderTypeValue("Content-Type"));
			}
			if (bodySizeIsValid(getServerByClientSocket(fd), uri, _clients[fd]->getLastFilePath()))
			{
				std::ifstream file(_clients[fd]->getLastFilePath());
				std::string line;
				if (file.is_open())
				{
					while (getline(file, line))
						content += line + "\r\n";
					file.close();
				}
				else
					std::cerr << "Impossible d'ouvrir le fichier" << std::endl;
			}
			else
				response.setErrorResponse(413, "Payload Too Large");
		}
		else if (_clients[fd]->getRequestProtocol() == "DELETE") {
			std::string resource = _clients[fd]->getRequestUri();
			// Get the base directory of the application
			char actualpath [PATH_MAX+1];
			realpath(__FILE__, actualpath);
			std::string baseDirectory = dirname(dirname(actualpath));

			// Concatenate the base directory with the resource
			std::string fullPath = baseDirectory + resource;

			// If the deletion was successful, send a 200 OK response.
			// Otherwise, send a 404 Not Found or 500 Internal Server Error response.
			if (deleteResource(fullPath)) {
				response.setDeleteResponse(200, "OK");
			} else {
				response.setDeleteResponse(404, "Not Found");
			}
		}
		else if (_clients[fd]->getRequestProtocol() == "POST" && Ft::endsWith(_clients[fd]->getRequestUri(), ".py")) {
			std::string scriptName = _clients[fd]->getLastFilePath();
			handleCGIRequest(*_clients[fd], scriptName);
			return true;
		}
		else
			response.setErrorResponse(500, "Internal Server Error");
	}
	if (content.empty())
	{
		if (_clients[fd]->getRequestProtocol() != "DELETE") {
			Server server = getServerByClientSocket(fd);
			for (std::map<int, std::string>::iterator it = server.getErrorPages().begin(); it != server.getErrorPages().end(); it++)
			{
				if (it->first == 404 && response.getStatusCode() != 413 && response.getStatusCode() != 405 && response.getStatusCode() != 501 && response.getStatusCode() != 500)
				{
					content = getResourceContent(it->second, fd);
					response.setNormalResponse(it->first, "Not Found", content, getMimeType(it->second), it->second);
					break;
				}
			}
			if (content.empty() && response.getStatusCode() != 413 && response.getStatusCode() != 405 && response.getStatusCode() != 501 && response.getStatusCode() != 500)
				response.setErrorResponse(404, "Not Found");
		}
	}
	else
	{
		std::string mimeType = getMimeType(uri);
		if (_clients[fd]->getRequestProtocol() == "GET")
			response.setNormalResponse(200, "OK", content, mimeType, _clients[fd]->getLastFilePath());
		else if (_clients[fd]->getRequestProtocol() == "POST")
			response.setNormalResponse(302, "Found", content, mimeType, _clients[fd]->getLastFilePath());
	}
	_clients[fd]->setClientResponse(response);
	FD_CLR(fd, &_writefds);

	// Utilisation de sendall pour envoyer toutes les données
	bool n = send(fd, response.getResponse().c_str(), response.getResponse().length());
	Ft::printLogs(getServerByClientSocket(fd), *_clients[fd], RESPONSE);
	return (n);
}

void MasterServer::handleCGIRequest(Client &client, std::string scriptName) {
    int 		pipefd[2];
    pid_t 		pid;
    char 		buf;
    std::size_t pos= client.getRequestUri().find("?");

	if (!Ft::fileExists(scriptName))
	{
		HttpResponse response;
        response.setErrorResponse(502, "Bad Gateway");
        client.setClientResponse(response);
		std::cout << client.getBodyPayload() << std::endl;
		return;
	}
    if (pipe(pipefd) == -1)
		exit(Ft::printErr("pipe failed.", NULL, EXIT_FAILURE, getPorts(), getClients()));

    pid = fork();
    if (pid == -1)
        exit(Ft::printErr("fork failed.", NULL, EXIT_FAILURE, getPorts(), getClients()));

    if (pid == 0) { // Child process
        alarm(7);
		std::string queryString;
        std::string requestMethod = "REQUEST_METHOD=" + client.getRequestProtocol();
		if (client.getRequestProtocol() == "GET")
		{
			queryString = "QUERY_STRING=" + client.getRequestUri().substr(pos + 1);
			std::cout << "QUERY STRING: " << queryString << std::endl;
		}
		else if (client.getRequestProtocol() == "POST")
		{
			std::cout << "BODY PAYLOAD: " << client.getBodyPayload() << std::endl;
			queryString = "QUERY_STRING=" + client.getBodyPayload();
		}
        std::string contentLength = "CONTENT_LENGTH=" + std::to_string(client.getBodyPayload().size());
        std::string contentType = "CONTENT_TYPE=" + client.getHeaderTypeValue("Content-Type");
        char* envp[] = {
            const_cast<char *>(requestMethod.c_str()),
            const_cast<char *>(queryString.c_str()),
            const_cast<char *>(contentLength.c_str()),
            const_cast<char *>(contentType.c_str()),
            NULL
        };
		char* const argv[] = {
			const_cast<char*>(scriptName.c_str()),
			NULL
		};

        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        int pipefd_in[2];
        if (pipe(pipefd_in) == -1)
            exit(Ft::printErr("pipe failed.", NULL, EXIT_FAILURE, getPorts(), getClients()));

		ssize_t written = write(pipefd_in[1], client.getBodyPayload().c_str(), client.getBodyPayload().size());
		if (written == -1)
			exit(Ft::printErr("write failed.", NULL, EXIT_FAILURE, getPorts(), getClients()));
		else if (written == 0 || written > 0) {
			close(pipefd_in[1]);
			dup2(pipefd_in[0], STDIN_FILENO);
			close(pipefd_in[0]);
		}

        execve(scriptName.c_str(), const_cast<char* const*>(argv), envp);
		exit(Ft::printErr("execve failed.", NULL, EXIT_FAILURE, getPorts(), getClients()));
    }
    else { // Parent process
        close(pipefd[1]);

        std::string output;
		ssize_t     readBytes = read(pipefd[0], &buf, 1);
		while (true) {
			if (readBytes == -1) {
				HttpResponse response;
				response.setErrorResponse(500, "Internal Server Error");
				client.setClientResponse(response);
			}
			if (readBytes == 0)
				break;
			output += buf;
			readBytes = read(pipefd[0], &buf, 1);
		}

        int status;
        waitpid(pid, &status, 0);

        if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
            // Handle error: The child process was terminated by a SIGALRM signal
            // This means the Python CGI script was running for too long (more than 5 seconds)
            HttpResponse response;
            response.setNormalResponse(500, "Internal Server Error", "The server encountered an internal error. Probably an INFINITE LOOP", "text/html", client.getLastFilePath());
            client.setClientResponse(response);
        }
        else {
            HttpResponse response;
            response.setNormalResponse(200, "OK", output, "text/html", client.getLastFilePath());
            client.setClientResponse(response);
        }
    }
	std::cout << "RESPONSE BODY: " << client.getResponseBody() << std::endl;
}

bool MasterServer::deleteResource(const std::string &resource)
{
	return remove(resource.c_str()) == 0;
}

void MasterServer::saveFile(const int &fd, const std::string &fileData, const std::string &mimeType)
{
	std::cout << "FILEDATA: " << fileData << std::endl;
	std::cout << "MIMETYPE: " << mimeType << std::endl;
	// std::string directoryPath = getServerByClientSocket(fd).getLocations();
	std::string target = _clients[fd]->getRequestUri();
	Location loc = getServerByClientSocket(fd).getLocationByPath(target);

	std::string directoryPath = !loc.upload.empty() ? loc.upload : "./tmp/";

	std::string extension = getExtensionFromMimeType(mimeType);
	// Si le directory path existe pas il faut le créer
	if (!Ft::fileExists(directoryPath))
		if (mkdir(directoryPath.c_str(), 0755) == -1)
			exit(Ft::printErr("mkdir failed.", NULL, EXIT_FAILURE, getPorts(), getClients()));

	std::string filePath = directoryPath + "postedFile" + extension;

	_clients[fd]->setLastFilePath(filePath);

	std::ofstream fileStream(filePath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

	if (!fileStream)
		exit(Ft::printErr("creation @ failed.", "postedFile", EXIT_FAILURE, getPorts(), getClients()));

	fileStream.write(fileData.c_str(), fileData.size());

	if (!fileStream.good())
		std::cerr << "Erreur lors de l'écriture dans le fichier" << std::endl;

	fileStream.close();
}

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

	// Server &server = getServerBySocketPort(listen_fd);

	_clients[newfd] = new Client(newfd, their_addr, true, listen_fd);
	getServerBySocketPort(listen_fd).addClient(newfd);

	Ft::printLogs(getServerBySocketPort(listen_fd), *_clients[newfd], CONNEXION);
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
	std::map<int, Client *>::iterator itMaster = _clients.find(fd);

	if (itMaster != _clients.end())
	{
		Ft::printLogs(getServerByClientSocket(fd), *itMaster->second, DISCONNECT);
		for (std::vector<int>::iterator it = getServerByClientSocket(fd).getClients().begin(); it != getServerByClientSocket(fd).getClients().end(); ++it) {
        	if (*it == fd) {
            	getServerByClientSocket(fd).getClients().erase(it);
            	break;  // Important pour éviter des erreurs après l'effacement
        	}
    	}
		delete itMaster->second;
		_clients.erase(fd);
	}

	close(fd); 
	removeFd(fd);
}

void MasterServer::signalHandler(int signal)
{
    if (_masterServerPtr)
	{
        const std::map<int, Client *> &clients = _masterServerPtr->getClients();
        for (std::map<int, Client *>::const_iterator it = clients.begin(); it != clients.end(); it++)
		{
            delete it->second;
			close(it->first);
		}
		std::cout << "iao Bye Bye <3" << std::endl;
		for (std::map<std::string, int>::iterator it = _masterServerPtr->getPorts().begin(); it != _masterServerPtr->getPorts().end(); it++)
		{
			close(it->second);
			std::cout << "Stop listening on port " << it->first << std::endl;
		}
        exit(signal);
    }
}