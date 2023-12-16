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
