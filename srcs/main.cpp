#include "../includes/Parsor.hpp"

int main(int argc, char **argv) {
	if (argc > 2)
        return Ft::print("Error: bad arg", NULL, EXIT_FAILURE);

    std::string fileConf = argv[1] ? argv[1] : "file.conf";
    if (!Parsor::parse(fileConf))
        return (EXIT_FAILURE);

	return 0;
}
