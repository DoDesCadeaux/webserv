#include "../includes/Parsor.hpp"
#include "../includes/MasterServer.hpp"

int main(int argc, char **argv)
{
	if (argc > 2)
		return Ft::print("Error: bad arg", NULL, EXIT_FAILURE);
	std::string fileConf = argv[1] ? argv[1] : "file.conf";
	if (!Parsor::parseIntegrity(fileConf))
		return (EXIT_FAILURE);

	MasterServer masterServer = Parsor::parse(fileConf);

	MasterServer::initializeMasterServer(&masterServer);

    signal(SIGINT, MasterServer::signalHandler);
    masterServer.run();
}
