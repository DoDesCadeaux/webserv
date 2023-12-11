#include "../includes/Parsor.hpp"
#include "../includes/Server.hpp"

int main(int argc, char **argv) {
	if (argc > 2)
		return Ft::print("Error: bad arg", NULL, EXIT_FAILURE);
	std::string fileConf = argv[1] ? argv[1] : "file.conf";
	if (!Parsor::parse(fileConf))
		return (EXIT_FAILURE);

	Server server;
	server.setSocket();
	int i = 0;
	while (true) {
		std::cout << "tour " << i << std::endl;
		server.run();
		i++;
	}
	return 0;
}
