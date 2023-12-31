#include "../includes/Parsor.hpp"
#include "../includes/Server.hpp"

using namespace std;

int main(int argc, char **argv)
{
	if (argc > 2)
		return Ft::print("Error: bad arg", NULL, EXIT_FAILURE);
	std::string fileConf = argv[1] ? argv[1] : "file.conf";
	if (!Parsor::parseIntegrity(fileConf))
		return (EXIT_FAILURE);

	std::vector<Server> servers = Parsor::parse(fileConf);

	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		it->run();
	}
}
