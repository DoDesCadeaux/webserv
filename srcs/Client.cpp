#include "../includes/Client.hpp"

Client::Client(int fd, struct sockaddr_storage addr, bool connect, int fdport) {
    _fd = fd;
    _addr = addr;
    _connect = connect;
    _fdport = fdport;
}

void Client::setClientRequest(const Request &requesttoset) {
	_clientrequest = requesttoset;
}

int Client::getFdPort() const {
    return _fdport;
}

int Client::getFd() const {
    return _fd;
}
