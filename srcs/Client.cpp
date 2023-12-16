#include "../includes/Client.hpp"

Client::Client(int fd, struct sockaddr_storage addr, bool connect, int fdport) {
    _fd = fd;
    _addr = addr;
    _connect = connect;
    _fdport = fdport;
}

const std::string &Client::getRequestLine() const {
	return _clientrequest.getLineRequest();
}

const std::string &Client::getRequestProtocol() const {
	return _clientrequest.getLineRequest();
}

const std::string &Client::getRequestUri() const {
	return _clientrequest.getUri();
}

const std::string &Client::getRequestFormat() const {
	return _clientrequest.getFormat();
}

void Client::setClientRequest(const Request &requesttoset) {
	_clientrequest = requesttoset;
}

const int &Client::getFdPort() const {
    return _fdport;
}

const int &Client::getFd() const {
    return _fd;
}
