#ifndef CLIENT_HPP
# define CLIENT_HPP
# define KEEP_ALIVE_TIMEOUT 5

#include <iostream>
#include <sys/socket.h>
#include "Request.hpp"
#include "HttpResponse.hpp"


class Client{
private:
	int						_fd;
	struct sockaddr_storage	_addr;
	bool					_connect;
	int						_fdport;
	Request					_clientrequest;
	bool 					_keepalive;
	time_t					_lastactivity;
	HttpResponse			_clientresponse;
	std::string 			_lastfilepath;

public:
	Client(int fd, struct sockaddr_storage addr, bool connect, int fdport);
	void			setClientRequest(const Request &requesttoset);
	void			setClientResponse(const HttpResponse &response);
	const int		&getFdPort() const;
	const int		&getFd() const;

	Request		&getRequest();
	const std::string 	&getRequestLine() const;
	const std::string 	&getRequestProtocol() const;
	const std::string 	&getRequestUri() const;
	const std::string 	&getRequestFormat() const;
	const std::string 	&getBodyPayload() const;
	const std::string 	&getLastFilePath() const;
	std::string 		getHeaderTypeValue(const std::string &headertype) const;

	const std::string	&getResponse() const;
	const int			&getResponseStatusCode () const;
	const std::string 	&getResponseStatusMsg() const;
	const unsigned long &getResponseLength() const;

	void setKeepAlive(bool ka);

	void setLastFilePath(const std::string &filepath);

	bool isKeepAlive() const;

	void resetKeepAliveTimer();

	bool hasKeepAliveTimedOut(int timeoutSeconds) const;
};

#endif
