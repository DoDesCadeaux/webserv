#include "../includes/Parsor.hpp"
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#define PORT "7812"

int main(int argc, char **argv) {
	if (argc > 2)
		return Ft::print("Error: bad arg", NULL, EXIT_FAILURE);
	std::string fileConf = argv[1] ? argv[1] : "file.conf";
	if (!Parsor::parse(fileConf))
		return (EXIT_FAILURE);

	struct addrinfo hint, *servinfo;
	memset(&hint, 0, sizeof(hint));

	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, PORT, &hint, &servinfo) != 0) {
		perror("Address Info");
		exit(EXIT_FAILURE);
	}

	int server_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol) ;
	if (server_fd == -1) {
		perror("Socket");
		exit(EXIT_FAILURE);
	}

	std::cout << server_fd << std::endl;

	fcntl(server_fd, F_SETFL, O_NONBLOCK);

	if (bind(server_fd, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
		perror("Bind");
		exit(EXIT_FAILURE);
	}

	int yes = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes) ) == -1) {
		perror("Setsockopt");
		exit(1);
	}

	return 0;
}
