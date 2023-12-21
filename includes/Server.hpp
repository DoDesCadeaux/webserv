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

	void							setServerName(std::string const &name);

};

#endif
