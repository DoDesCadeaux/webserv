#include "../includes/Request.hpp"

Request::Request() {}

Request::Request(const std::string &requestformat) : _requestformat(requestformat) {
	setupRequest();
}

void Request::setLine() {
	ssize_t endl = _requestformat.find('\r');
	_requestline = _requestformat.substr(0, endl);
}

void Request::setProtocol() {
	ssize_t endProtocol = _requestline.find(' ');
	_requestprotocol = _requestline.substr(0, endProtocol);
}

void Request::setUri() {
	ssize_t uripos = _requestline.find('/');
	std::string uritoend;

	uritoend = _requestline.substr(uripos, std::string::npos);

	ssize_t spacepos = uritoend.find(' ');

	_requesturi = uritoend.substr(0, spacepos);
}

void Request::setHeader() {
	if (_requestprotocol == "GET") {
		std::istringstream	stream(_requestformat);
		std::string 		line;

		std::getline(stream, line);
		while(std::getline(stream, line) && line != "\r") {
			std::istringstream	linestream(line);
			std::string			key, value;

			if (std::getline(linestream, key, ':')) {
				if (std::getline(linestream, value)) {
					if (!value.empty() && value[0] == ' ')
						value.erase(0, 1);
				}
				_requestheadertypes[key] = value;
			}
		}
	}
}

void Request::setupRequest() {
	this->setLine();
	this->setProtocol();
	this->setUri();
	this->setHeader();
}

void Request::displayHeaderTypes() const {
	for (std::map<std::string, std::string>::const_iterator it = _requestheadertypes.begin(); it != _requestheadertypes.end(); ++it) {
		std::cout << it->first << ":" << it->second << std::endl;
	}
}

bool Request::isValidProtocol() {
	std::vector<std::string> vect;
	vect.push_back("GET");
	vect.push_back("DELETE");
	vect.push_back("POST");

	std::vector<std::string>::iterator it;
	for (it = vect.begin(); it != vect.end() ; ++it) {
		if (_requestprotocol == *it)
			break;
	}
	if (it == vect.end())
		return false;
	return true;
}

void Request::replaceProtocolTo(const std::string &newprotocol) {
	_requestprotocol = newprotocol;
}

const std::string &Request::getFormat() const {
	return _requestformat;
}

const std::string &Request::getLineRequest() const {
	return _requestline;
}

const std::string &Request::getProtocol() const {
	return _requestprotocol;
}

const std::string &Request::getUri() const {
	return _requesturi;
}
