#include "../includes/Server.hpp"

// CONSTRUCTOR - DESTRUCTOR
Server::Server()
{
	//Quid de l'utilité
	// FD_ZERO(&_allfds);
	// FD_ZERO(&_readfds);
	// FD_ZERO(&_writefds);
	// _listfds.clear();
	// _name = "localhost";
}

Server	&Server::operator=(const Server &other)
{
	_ports = other._ports;
	_name = other._name;
	_root = other._root;
    _errorPage = other._errorPage;
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

std::vector<Location> &Server::getLocations()
{
	return _locations;
}

std::vector<int> &Server::getClients(){
	return _clients;
}

// SETTER
void	Server::setServerName(std::string const &name){
	_name = name;
}
