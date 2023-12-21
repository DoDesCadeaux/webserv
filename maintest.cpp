#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <cstring>
#include <sys/stat.h>

#define ROOT "root"
#define SERV_NAME "server_name"
#define LISTEN "listen"
#define ERR_PAGE "error_page"
#define LOCATION "location"
#define SERVER "server"


void announceError(std::string location, std::string type)
{
    if (location.empty())
        std::cout << type << ": error: ";
    else
        std::cout << type << " " << location << ": error: ";
}

template <typename T>
bool alreadySet(T &toSave, std::string directive, std::string type, std::string location)
{
    if (!toSave.empty())
    {
        announceError(location, type);
        std::cout << directive << " is already set." << std::endl;
        return false;
    }
    return true;
}

template <typename T>
bool goodArg(T &toSave, std::istringstream &iss, std::string directive, std::string type, std::string location)
{
    removeSemicolon(iss) >> toSave;
    std::string additionalValue = checkNbParam(0, iss);
    if (!additionalValue.empty())
    {
        announceError(location, type);
        std::cout << "too many param for " << directive << " directive" << std::endl;
        return false;
    }
    if (toSave.empty())
    {
        announceError(location, type);
        std::cout << "missing param for " << directive << " directive" << std::endl;
        return false;
    }
    return true;
}

template <typename T>
bool directiveIsValid(T &toSave, std::istringstream &iss, std::string directive, std::string type, std::string location)
{
    if (directive == "root" || directive == "server_name" || directive == "index" || directive == "autoindex" || directive == "auth" || directive == "upload")
    {
        if (!alreadySet(toSave, directive, type, location))
            return false;
        if (!goodArg(toSave, iss, directive, type, location))
            return false;
    }
    if (directive == "autoindex" && toSave != "on" && toSave != "off")
    {
        announceError(location, type);
        std::cerr << "autoindex directive must be set to on/off" << std::endl;
        return false;
    }
    if (directive == "root")
    {
        toSave = startsWith(toSave, "./") ? toSave : (startsWith(toSave, "/") ? "." + toSave : "./" + toSave);
        if (!pathExists(toSave))
        {
            announceError(location, type);
            std::cout << "invalid root " << toSave << std::endl;
            return false;
        }
    }

    return true;
}


Server parsing(std::string fileName)
{
    std::ifstream inputFile(fileName);

    if (!inputFile.is_open())
    {
        std::cerr << "Erreur lors de l'ouverture du fichier." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    std::vector<Server> servers;
    Server currentServer;
    Location currentLocation;

    while (std::getline(inputFile, line))
    {
        std::istringstream iss(line);
        std::string token;

        iss >> token;

        if (token == "server{")
            currentServer = Server();
        else if (token == "root" && !directiveIsValid(currentServer.root, iss, token, SERVER, ""))
            exit(EXIT_FAILURE);
        else if (token == "server_name" && !directiveIsValid(currentServer.serverName, iss, token, SERVER, ""))
            exit(EXIT_FAILURE);
        // else if (token == "listen")
        // {
        //     // Récupérer le port d'écoute
        //     std::string listenPort;
        //     removeSemicolon(iss) >> listenPort;
        //     std::string additionalValue = checkNbParam(0, iss);
        //     if (!additionalValue.empty())
        //     {
        //         std::cout << "Server :Error: too many param for listen directive" << std::endl;
        //         return 1;
        //     }
        //     // Vérifier si le port d'écoute existe déjà
        //     if (currentServer.listenPorts.find(listenPort) != currentServer.listenPorts.end())
        //     {
        //         std::cout << "Server :Error: port " << listenPort << " already set" << std::endl;
        //         return 1;
        //     }
        //     if (listenPort.empty())
        //     {
        //         std::cout << "Server :Error: missing param for listen directive" << std::endl;
        //         return 1;
        //     }
        //     if (!isNumeric(listenPort))
        //     {
        //         std::cout << "Server :Error: port " << listenPort << " is not a valid port" << std::endl;
        //         return 1;
        //     }

        //     // ICI -> introduire la vérification de port si deja sur ecoute

        //     // Ajouter le port d'écoute au serveur
        //     currentServer.listenPorts.insert(listenPort);
        // }
        else if (token == "error_page")
        {
            // Vérifier si error_page a déjà été initialisé
            // Quid de l'utilité on peux avoir error page 503 maybe
            if (!currentServer.errorPage.empty())
            {
                std::cout << "Server :Error: error_page directive is already set" << std::endl;
                exit(EXIT_FAILURE);
            }

            // Récupérer la page d'erreur
            std::string errorPage;
            iss >> errorPage;

            std::string additionalValue = checkNbParam(1, iss);
            if (additionalValue.empty())
            {
                std::cout << "Server :Error: error_page directive has to have ONE param" << std::endl;
                exit(EXIT_FAILURE);
            }

            // Ajouter la paire à la map si errorPage est un nombre ATENTION VERIFIER LE PATH
            if (!isNumeric(errorPage))
            {
                std::cout << "Server :Error: error_page directive must be an integer" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (!pathExists("./" + additionalValue))
            {
                std::cout << "Server :Error: " << additionalValue << " invalid path for error_page directive " << errorPage << std::endl;
                // return 1;
            }
            currentServer.errorPage[std::atoi(errorPage.c_str())] = removeSemicolon(additionalValue);
        }
        else if (token == "location")
        {
            currentLocation = Location();
            // Nouvelle location ATENTION VOIR POUR LE PATH EN ENTIER
            iss >> currentLocation.path;
            std::string directive;
            while (std::getline(inputFile, line))
            {
                std::istringstream locIss(line);
                locIss >> directive;

                // Ajouter check dans la liste des token directives
                if (directive == "}")
                    break;
                else if (directive == "root" && !directiveIsValid(currentLocation.root, locIss, directive, LOCATION, currentLocation.path))
                    exit(EXIT_FAILURE);
                else if (directive == "index" && !directiveIsValid(currentLocation.index, locIss, directive, LOCATION, currentLocation.path))
                    exit(EXIT_FAILURE);
                else if (directive == "autoindex" && !directiveIsValid(currentLocation.autoindex, locIss, directive, LOCATION, currentLocation.path))
                    exit(EXIT_FAILURE);
                else if (directive == "cgi")
                {
                    std::string param;
                    removeSemicolon(locIss) >> param;
                    std::string additionalValue = checkNbParam(1, locIss);
                    if (additionalValue.empty())
                    {
                        std::cerr << "Erreur: La directive cgi doit avoir exactement un paramètre supplémentaire." << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    currentLocation.cgi[param] = removeSemicolon(additionalValue);
                }
                else if (directive == "auth" && !directiveIsValid(currentLocation.auth, locIss, directive, LOCATION, currentLocation.path))
                    exit(EXIT_FAILURE);
                else if (directive == "upload" && !directiveIsValid(currentLocation.upload, locIss, directive, LOCATION, currentLocation.path))
                    exit(EXIT_FAILURE);
                else
                {
                    // limit_except;
                    // client_max_body_size
                }
            }
            currentServer.locations.push_back(currentLocation);
        }
        else if (token == "}")
            servers.push_back(currentServer);
    }

    // Vérification
    if (servers.empty())
    {
        std::cout << "Missing server bloc" << std::endl;
        exit(EXIT_FAILURE);
    }

    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
    {
        if (it->serverName.empty())
            it->serverName = "localhost";
    }
// std::cout << "==========================================\n";
    // // Traitement final
    // for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
    // {
    //     // Root
    //     std::cout << "Server \n- Root: " << it->root << std::endl;
    //     // Server Name
    //     std::cout << "- Server name: " << it->serverName << std::endl;
    //     // Ports
    //     std::cout << "- Listen Ports: ";
    //     for (std::set<std::string>::iterator portIt = it->listenPorts.begin(); portIt != it->listenPorts.end(); ++portIt)
    //     {
    //         std::cout << *portIt << " ";
    //     }
    //     std::cout << std::endl;
    //     // Error Page
    //     for (std::map<int, std::string>::iterator errIt = it->errorPage.begin(); errIt != it->errorPage.end(); ++errIt)
    //     {
    //         std::cout << "- Error Page: ";
    //         std::cout << errIt->first << " " << errIt->second;
    //     }
    //     std::cout << std::endl;
    //     // Locations
    //     for (std::vector<Location>::iterator locIt = it->locations.begin(); locIt != it->locations.end(); ++locIt)
    //     {
    //         std::cout << "- Location" << std::endl;
    //         ;
    //         std::cout << "\t- Path: " << locIt->path << std::endl;
    //         if (!locIt->root.empty())
    //             std::cout << "\t- Root: " << locIt->root << std::endl;
    //         if (!locIt->index.empty())
    //             std::cout << "\t- Index: " << locIt->index << std::endl;
    //         if (!locIt->autoindex.empty())
    //             std::cout << "\t- AutoIndex: " << locIt->autoindex << std::endl;
    //         if (!locIt->cgi.empty())
    //         {
    //             for (std::map<std::string, std::string>::iterator cgiIt = locIt->cgi.begin(); cgiIt != locIt->cgi.end(); ++cgiIt)
    //             {
    //                 std::cout << "\t- cgi: ";
    //                 std::cout << cgiIt->first << " " << cgiIt->second;
    //             }
    //             std::cout << std::endl;
    //         }

    //         if (!locIt->limit_except.empty())
    //             std::cout << "\t- limit_except: " << locIt->limit_except << std::endl;
    //         if (!locIt->auth.empty())
    //             std::cout << "\t- auth: " << locIt->auth << std::endl;
    //         if (!locIt->upload.empty())
    //             std::cout << "\t- upload: " << locIt->upload << std::endl;
    //     }
    // }

    return servers[0];
}


int main()
{
    Server server = parsing("file.conf");

    return 0;
}
