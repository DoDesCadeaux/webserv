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
///CGI
# define CGI_SCRIPT_PATH "web/cgi-bin/..."
# define CGI_UPLOAD_SCRIPT_PATH "/form"
///
# define SERVER_ROOT "web"
# define GREEN "\e[32m"
# define NOCOL "\e[39m"
# define RED "\e[91m"

# define CONNEXION "New connexion"
# define DISCONNECT "Connexion closed"
# define REQUEST "Request"
# define RESPONSE "Response"

class Client;

struct Location
{
    std::string path;
    std::string root;
    std::string index;
    std::string autoindex;
    std::map<std::string, std::string> cgi;
    std::string limit_except;
    std::string auth;
    std::string upload;
    Location() : path(""), root(""), index(""), autoindex(""), cgi(), limit_except(""), auth(""), upload("")
    {
    }
};

class Server
{
private:
	std::map<std::string, int>		_ports;
	std::string						_name;
	std::string						_root;
    std::map<int, std::string>		_errorPage;
	std::vector<Location> 			_locations;
	std::list<int>					_listfds;
	fd_set							_allfds;
	fd_set							_readfds;
	fd_set							_writefds;
	int 							_maxfd;
	std::map<int, Client*>			_clients;

public:
	Server();
	Server &operator=(const Server &other);
	~Server();

	void				saveFile(const int &fd, const std::string &imageData, const std::string &directoryPath, const std::string &mimeType);
	static	std::string getExtensionFromMimeType(const std::string &mimeType);

	void 						setSocket();
	void						addFd(int fd);
	void						addClientFd(int fd);
	void						removeFd(int fd);
	void						run();
	bool						sendAll(const int &fd);
	bool						recvAll(const int &fd);
	void						newConnection(const int &fd);
	void						killConnection(const int &fd);
	void						saveImage(const std::string& imageData, const std::string& filePath);
	static	std::string			generateRandomFileName(const std::string& extension);
	static	std::string 		getResourceContent(const std::string &uri);
	static	std::string			getMimeType(const std::string& uri);
	
	std::string 				&getServerName();
	std::string 				&getRoot();
	std::map<std::string, int>	&getPorts();
	std::vector<Location>		&getLocations();
	void						setServerName(std::string const &name);

	////////CGI
	bool						isCGIRequest(const Request &request);
	bool						handleCGIRequest(int fd);

};

#endif
