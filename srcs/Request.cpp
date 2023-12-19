#include "../includes/Request.hpp"

Request::Request() {};

Request::Request(const std::string &requestformat) : _requestformat(requestformat) {
	setupRequest();
}

Request &Request::operator=(const Request &other) {
	if (this != &other) {
		_requestline = other._requestline;
		_requestprotocol = other._requestprotocol;
		_requesturi = other._requesturi;
		_requestformat = other._requestformat;
		_requestheadertypes = other._requestheadertypes;
		_requestbody = other._requestbody;
	}

	return *this;
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

void Request::setGetRequest() {
	std::istringstream	stream(_requestformat);
	std::string 		line;

	std::getline(stream, line);
	while (std::getline(stream, line) && line != "\r") {
		std::istringstream	linestream(line);
		std::string			key, value;

		if (std::getline(linestream, key, ':')) {
			if (std::getline(linestream, value))
				if (!value.empty() && value[0] == ' ')
					value.erase(0, 1);
			_requestheadertypes[key] = value;
		}
	}
}

void Request::setPostRequest() {
	// Trouver la fin des en-têtes HTTP
	std::string::size_type headerEnd = _requestformat.find("\r\n\r\n");

	if (headerEnd != std::string::npos) {
		// Garder les en-têtes tels quels
		_requestbody = _requestformat.substr(headerEnd + 4); // Extraire le payload

		std::istringstream stream(_requestbody);
		std::string line;
		std::string processedPayload;
		bool isFirstValidLine = true;

		while (std::getline(stream, line)) {
			// Vérifier les occurrences dans le payload
			if (line.find("----") != std::string::npos ||
				line.find("Content") != std::string::npos ||
				line.find("Envoyer") != std::string::npos) {
				continue; // Ignorer cette ligne
			}
			// Vérifier si c'est la première ligne valide
			if ((isFirstValidLine && line.empty()) || (isFirstValidLine && line == "\r")) {
				continue; // Ignorer les lignes vides au début
			}

			isFirstValidLine = false;

			// Ajouter la ligne au payload traité
			processedPayload += line + "\n";
		}

		// Supprimer les lignes vides de fin si elles existent
		std::string::size_type endPos = processedPayload.find_last_not_of("\r\n");
		if (endPos != std::string::npos) {
			processedPayload = processedPayload.substr(0, endPos + 1);
		}

		// Mettre à jour le payload avec les lignes traitées
		_requestbody = processedPayload;
	}
}



void Request::setPayload(const std::string &payload) {
	_requestbody = payload;
}

void Request::setHeader() {
	if (_requestprotocol == "GET")
		setGetRequest();
	else if (_requestprotocol == "POST") {
		setPostRequest();
		displayHeaderTypes();
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

const std::string &Request::getBodyPayload() const {
	return _requestbody;
}

const std::string Request::getHeader(const std::string &headertype) const {
	std::map<std::string, std::string>::const_iterator it = _requestheadertypes.find(headertype);
	if (it != _requestheadertypes.end())
		return it->second;
	else
		return std::string(" ");
}
