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

# define BUFFER_SIZE 4096 //test values under 4
# define SERVER_ROOT "web"
# define GREEN "\e[32m"
# define NOCOL "\e[39m"
# define RED "\e[91m"

class Client;

class Server
{
	private:
		std::list<char *>				_ports;
		std::list<int>					_listfds;
		fd_set							_allfds;
		fd_set							_readfds;
		fd_set							_writefds;
		int 							_maxfd;
		std::map<int, Client*>			_clients;
		//	std::vector<int>				_clients;
		// container? workers; => List de res de socket()
		// container? client: => List de res de accept()

	public:
		Server();
		~Server();
		void 	setSocket();
		void	addFd(int fd);
		void	addClientFd(int fd);
		void	removeFd(int fd);
		void	run();
		int		sendAll(const int &fd,  const std::string &httpResponse, unsigned int *len);
		int		recvAll(const int &fd);
		void	newConnection(const int &fd);
		void	killConnection(const int &fd);
		std::string	getResourceContent(const int &fd);
		static std::string	getMimeType(const std::string& uri);
		// void	accept();
		// kill conmnexion
		// run server avec le select
		// revc
		// send
		// update fd_set
};

#endif
