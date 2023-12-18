#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Parsor.hpp"
#include "Request.hpp"
#include "HttpResponse.hpp"
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <list>
#include <map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>

# define BUFFER_SIZE 1024 //test values under 4
# define SERVER_ROOT "web"
# define GREEN "\e[32m"
# define NOCOL "\e[39m"
# define RED "\e[91m"

# define CONNEXION "New connexion"
# define DISCONNECT "Connexion closed"
# define REQUEST ""

class Client;

class Server
{
private:
	std::map<std::string, int>			_ports;
	std::list<int>					_listfds;
	fd_set							_allfds;
	fd_set							_readfds;
	fd_set							_writefds;
	int 							_maxfd;
	std::map<int, Client*>			_clients;
	std::string						_name;

public:
	Server();
	~Server();
	void 	setSocket();
	void	addFd(int fd);
	void	addClientFd(int fd);
	void	removeFd(int fd);
	void	run();
	bool	sendAll(const int &fd);
	bool	recvAll(const int &fd);
	void	newConnection(const int &fd);
	void	killConnection(const int &fd);
	void	saveImage(const std::string& imageData, const std::string& filePath);
	static	std::string	generateRandomFileName(const std::string& extension);
	static	std::string getResourceContent(const std::string &uri);
	static	std::string	getMimeType(const std::string& uri);
	std::string &getServerName();
	std::map<std::string, int> &getPorts();

};

#endif
