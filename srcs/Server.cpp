#include "../includes/Server.hpp"

// CONSTRUCTOR - DESTRUCTOR
Server::Server()
{
	// Quid de l'utilité
	// _errorPages.clear();
	_ports.clear();
}

Server &Server::operator=(const Server &other)
{
	_ports = other._ports;
	_name = other._name;
	_root = other._root;
	_errorPages = other._errorPages;
	_locations = other._locations;
	// _listfds = other._listfds;
	// _allfds = other._allfds;
	// _readfds = other._readfds;
	// _writefds = other._writefds;
	// _maxfd = other._maxfd;
	_clients = other._clients;
	return (*this);
}

Server::~Server() {}

// METHODS
void Server::addClient(const int &fd)
{
	_clients.push_back(fd);
}

// GETTER
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

std::map<int, std::string> &Server::getErrorPages()
{
	return _errorPages;
}

std::vector<Location> &Server::getLocations()
{
	return _locations;
}

std::vector<int> &Server::getClients()
{
	return _clients;
}

Location &Server::getLocationByPath(const std::string &path)
{
	for (std::vector<Location>::iterator it = _locations.begin(); it != _locations.end(); it++)
	{
		if (it->path == path)
			return *it;
	}
	throw std::runtime_error("Location not found");
}

bool Server::isAuthorizedProtocol(std::string path, const std::string &protocol)
{
	try
	{
		Location location = getLocationByPath(path);
		std::cout << "Je trouve Le location et tout vas bien avec le path " << path << std::endl;

		for (std::vector<std::string>::iterator it = location.limit_except.begin(); it != location.limit_except.end(); it++)
		{
			std::cout << COL << "it->" << *it << std::endl;
		}
		std::cout << NOCOL;
		if (location.limit_except.empty())
		{
			std::cout << "Limit est vide\n";
			return true;
		}
		for (std::vector<std::string>::iterator it = location.limit_except.begin(); it != location.limit_except.end(); it++)
		{
			std::cout << *it << "avec protocol " << protocol << std::endl;
			if (protocol == *it)
				return true;
		}
		return false;
	}
	catch (const std::exception &e)
	{
		std::cout << "J'ai pas trouvé de location\n";
		return true;
	}
}

// SETTER
void Server::setServerName(std::string const &name)
{
	_name = name;
}
