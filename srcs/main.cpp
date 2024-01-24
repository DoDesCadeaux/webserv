#include "../includes/Parsor.hpp"
#include "../includes/MasterServer.hpp"

int main(int argc, char **argv)
{
	if (argc > 2)
		return Ft::printErr("too much arguments", NULL, EXIT_FAILURE, "", NULL);
		
	std::string fileConf = argv[1] ? argv[1] : "file.conf";
	Parsor::parseIntegrity(fileConf);

	MasterServer masterServer = Parsor::parse(fileConf);

	MasterServer::initializeMasterServer(&masterServer);

    // signal(SIGINT, MasterServer::signalHandler);
    masterServer.run();
}
