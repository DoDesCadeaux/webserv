#include "../includes/Ft.hpp"

void Ft::printSet(const fd_set &set, const std::string &name) {
    std::cout << name << " : { ";

    for (int i = 0; i < FD_SETSIZE; ++i) {
        if (FD_ISSET(i, &set)) {
            std::cout << i << " ";
        }
    }
    std::cout << "}" << std::endl;
}

void Ft::printClient(const std::map<int, Client *> &clients) {
	std::cout << "clients : [ ";
	for (std::map<int, Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		std::cout << it->first << " ";
	}
	std::cout << "]" << std::endl;
}

bool Ft::fileExists(const std::string &filePath) {
	std::ifstream file(filePath.c_str());
	return file.good();
}

bool Ft::endsWith(const std::string& str, const std::string& suffix) {
	if (str.length() < suffix.length()) {
		return false;
	}
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

void Ft::printLogs(Server &serv, Client &client, std::string type){
	std::string port;
	for (std::map<std::string, int>::const_iterator it = serv.getPorts().begin(); it != serv.getPorts().end(); it++){
		if (it->second == client.getFdPort()){
			port = it->first;
		}
	}

	std::cout << "[" << serv.getServerName() << ":" << port << ":" << client.getFd() << "] ";

	if (type == CONNEXION)
		std::cout << GREEN << CONNEXION << NOCOL << std::endl;
	else if (type == DISCONNECT)
		std::cout << RED << DISCONNECT << NOCOL << std::endl;
	else if (type == REQUEST)
		std::cout << "<< [" << client.getRequestProtocol() << "] target:" << client.getRequestUri() << std::endl;
	else if (type == RESPONSE){
		if (client.getRequestProtocol() == "DELETE")
			std::cout << ">> [Status: " << client.getResponseStatusCode() << " " << client.getResponseStatusMsg() << "]" << std::endl;
		else
			std::cout << ">> [Status: " << client.getResponseStatusCode() << " " << client.getResponseStatusMsg() << "] length:" << client.getResponseLength() << std::endl;
	}
}

bool Ft::startsWith(const std::string &str, const std::string &prefix)
{
    return str.substr(0, prefix.size()) == prefix;
}

std::string Ft::to_string(int nb)
{
	std::stringstream ss;
    ss << nb;
    return ss.str();
}