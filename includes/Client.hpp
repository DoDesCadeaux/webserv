#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <sys/socket.h>
#include "../includes/Request.hpp"
#include "../includes/HttpResponse.hpp"

class Client{
private:
	int						_fd;
	struct sockaddr_storage	_addr;
	bool					_connect;
	int						_fdport;
	Request					_clientrequest;

public:
	Client(int fd, struct sockaddr_storage addr, bool connect, int fdport);
	void	setClientRequest(const Request &requesttoset);
	int		getFdPort() const;
	int		getFd() const;

};

#endif
