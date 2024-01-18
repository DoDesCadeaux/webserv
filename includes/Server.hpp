#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Parsor.hpp"
#include "Request.hpp"
#include "HttpResponse.hpp"
#include "cgi.hpp"
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
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>

# define BUFFER_SIZE 1024 //test values under 4
# define GREEN "\e[32m"
# define COL "\e[36m"
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
    std::string auth;
    std::string upload;
	std::string	max_body;
	std::vector<std::string> limit_except;
    std::map<std::string, std::string> cgi;

    Location() : path(""), root(""), index(""), autoindex(""), auth(""), upload(""), max_body(""), limit_except(), cgi()
    {
    }
};

class Server
{
private:
	std::string						_root;
	std::string						_name;
	std::map<std::string, int>		_ports;
    std::map<int, std::string>		_errorPages;
	std::vector<Location> 			_locations;
	std::vector<int>				_clients;

public:
	Server();
	Server &operator=(const Server &other);
	~Server();

	void							addClient(const int &fd);

	std::map<std::string, int>		&getPorts();
	std::string 					&getServerName();
	std::string 					&getRoot();
	std::vector<Location>			&getLocations();
	std::vector<int>				&getClients();
	std::map<int, std::string> 		&getErrorPages();
	Location 						&getLocationByPath(const std::string& path);
	bool							isAuthorizedProtocol(std::string path, const std::string &protocol);

	void							setServerName(std::string const &name);

};

#endif
