#include "../includes/Client.hpp"

Client::Client(int fd, struct sockaddr_storage addr, bool connect, int fdport) {
    _fd = fd;
    _addr = addr;
    _connect = connect;
    _fdport = fdport;
	_keepalive = false;
	_lastactivity = time(NULL);
}

const std::string &Client::getRequestLine() const {
	return _clientrequest.getLineRequest();
}

const std::string &Client::getRequestProtocol() const {
	return _clientrequest.getProtocol();
}

const std::string &Client::getRequestUri() const {
	return _clientrequest.getUri();
}

const std::string &Client::getRequestFormat() const {
	return _clientrequest.getFormat();
}

const std::string &Client::getBodyPayload() const {
	return _clientrequest.getBodyPayload();
}

void Client::setClientRequest(const Request &requesttoset) {
	_clientrequest = requesttoset;
}

void    Client::setClientResponse(const HttpResponse &response){
    _clientresponse = response;
}


const int &Client::getFdPort() const {
    return _fdport;
}

const int &Client::getFd() const {
    return _fd;
}

bool Client::isKeepAlive() const {
	return _keepalive;
}

void Client::setKeepAlive(bool ka) {
	_keepalive = ka;
}

void Client::resetKeepAliveTimer() {
	_lastactivity = time(NULL);
}

bool Client::hasKeepAliveTimedOut(int timeoutSeconds) const {
	time_t now = time(NULL);
	double elapsed = difftime(now, _lastactivity);
	return elapsed > timeoutSeconds;
}

const std::string &Client::getResponse() const{
	return _clientresponse.getResponse();
}

const int		&Client::getResponseStatusCode () const{
	return _clientresponse.getStatusCode();
}

const std::string &Client::getResponseStatusMsg() const{
	return _clientresponse.getStatusMessage();
}