/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsor.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamartin <pamartin@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/28 14:01:23 by pamartin          #+#    #+#             */
/*   Updated: 2023/11/28 14:01:26 by pamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Parsor.hpp"

// Fonction pour supprimer le ';' d'une chaîne
std::string removeSemicolon(const std::string &input)
{
    size_t pos = input.find(';');
    if (pos != std::string::npos)
    {
        return input.substr(0, pos);
    }
    return input;
}

// Classe adaptateur pour utiliser removeSemicolon avec l'opérateur >>
class RemoveSemicolonAdapter
{
public:
    RemoveSemicolonAdapter(std::istream &is) : is_(is) {}

    template <typename T>
    RemoveSemicolonAdapter &operator>>(T &val)
    {
        is_ >> token_;
        val = removeSemicolon(token_);
        return *this;
    }

private:
    std::istream &is_;
    std::string token_;
};

// Fonction utilitaire pour créer un adaptateur
RemoveSemicolonAdapter removeSemicolon(std::istream &is)
{
    return RemoveSemicolonAdapter(is);
}

/**
 * @brief Permet de trouver le fin de la ligne sans prendre en compte les commentaires
 * @example "workers{      " => return '{'
 * @example "workers; #pour mac" => return ';'
 *
 * @param line String ou l'on cherche le dernier char "utilisable"
 * @return Le dernier char "utilisable" de la ligne
 */
static char findEnd(std::string line)
{
    size_t stop = line.size();
    for (size_t i = 0; i < stop; i++)
    {
        if (line[i] == '#')
            stop = i;
    }
    for (size_t i = stop - 1; i >= 0; --i)
    {
        if (line[i] == ' ' || line[i] == '\t')
            continue;
        return line[i];
    }
    return '\0';
}

/**
 * @brief Permet de faire le parsing d'intégrité du fichier de configuration
 * Elements pris en compte :
 *      - Ouverture et fermeture des brackets
 *      - Ponctuation en fin de ligne
 * 		- Commentaires (ligne complete ou en fin de ligne)
 *
 * @param configName Nom du fichier de configuration
 * @return Un int indiquant si le parsing est ok ou non
 */
int Parsor::parseIntegrity(std::string configName)
{
    std::string line;
    std::ifstream file;
    std::vector<int> brackets;
    int itLine = 0;
    char last;

    file.open(configName);
    if (!file.is_open())
        return Ft::print("Error: failed to open @.", configName, 0);

    while (std::getline(file, line))
    {
        itLine++;
        last = findEnd(line);

        if (line.empty() || line[line.find_first_not_of(" \t", 0)] == '#' || last == '\0' || last == 4)
            continue;
        // Manque fonction check toute la ligne du debut jusqu'au last
        if (last == '{')
        {
            brackets.push_back(itLine);
            continue;
        }
        else if (last == '}')
        {
            if (brackets.empty())
                return Ft::print(ERR_CONF_EXTRA_BR, itLine, 0);
            brackets.pop_back();
            continue;
        }
        else if (last != ';')
        {
            return Ft::print(ERR_CONF_MISSING_PONCT, itLine, 0);
        }
    }
    if (!brackets.empty())
        return Ft::print(ERR_CONF_MISSING_BR, brackets.back(), 0);

    file.close();

    return 1;
}

bool isNumeric(const std::string &str)
{
    for (std::size_t i = 0; i < str.length(); ++i)
    {
        if (!std::isdigit(str[i]))
            return false;
    }
    return true;
}

static std::string checkNbParam(int nb, std::istringstream &iss)
{
    std::string additionalValue;
    std::string tmp;
    int i = 0;
    while (iss >> tmp)
    {
        if (tmp[0] == '#')
            break;
        i++;
        additionalValue = tmp;
    }
    if (i > nb && nb > 0)
        return "";
    return removeSemicolon(additionalValue);
}

void announceError(std::string location, std::string type)
{
    if (location.empty())
        std::cout << type << ": error: ";
    else
        std::cout << type << " " << location << ": error: ";
}

bool alreadySet(std::string &toSave, std::string directive, std::string type, std::string location)
{
    if (!toSave.empty())
    {
        announceError(location, type);
        std::cout << directive << " is already set." << std::endl;
        return false;
    }
    return true;
}

bool goodArg(std::string &toSave, std::istringstream &iss, std::string directive, std::string type, std::string location)
{
    removeSemicolon(iss) >> toSave;
    if (directive == "client_max_body_size")
    {
        if (!isNumeric(toSave))
        {
            announceError(location, type);
            std::cout << directive << " directive"
                      << "must be an integer" << std::endl;
            return false;
        }
    }

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

bool directiveIsValid(std::string &toSave, std::istringstream &iss, std::string directive, std::string type, std::string location)
{

    if (directive == "root" || directive == "server_name" || directive == "index" || directive == "autoindex" || directive == "auth" || directive == "upload" || directive == "client_max_body_size")
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
        toSave = Ft::startsWith(toSave, "./") ? toSave : (Ft::startsWith(toSave, "/") ? "." + toSave : "./" + toSave);
        if (!Ft::fileExists(toSave))
        {
            announceError(location, type);
            std::cout << "invalid root " << toSave << std::endl;
            return false;
        }
    }

    return true;
}

void setSocket(MasterServer &masterServer, Server &server, std::string port)
{
    struct addrinfo hint, *servinfo;
    int server_fd;
    int yes = 1;

    memset(&hint, 0, sizeof(hint)); // Pas oublier de free quand un truc fail après
    // Parametrage de la structure tampon (hint)
    hint.ai_family = AF_UNSPEC;     // Quelque soit l'ipv
    hint.ai_socktype = SOCK_STREAM; // precise type socket (streaming)
    hint.ai_flags = AI_PASSIVE;     // Assigner localhost au socket

    // Set up les infos du server correctement grace aux params de la struc tampon (hint)
    if (getaddrinfo(NULL, port.c_str(), &hint, &servinfo) != 0)
    {
        perror("Address Info");
        exit(EXIT_FAILURE);
    }

    // Mise en place du socket général
    server_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (server_fd == -1)
    {
        perror("Socket");
        exit(EXIT_FAILURE);
    }

    // Set up socket en non-bloquant
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    // En cas de re-run du server protection echec bind : "address already in use"
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        perror("Setsockopt");
        exit(EXIT_FAILURE);
    }

    // Attribution du socket au port
    if (bind(server_fd, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
    {
        perror("Bind");
        std::cout << "with port " << port << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1000) < 0)
    {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    // Free du addrinfo
    freeaddrinfo(servinfo);

    server.getPorts().insert(std::make_pair(port, server_fd));

    masterServer.addFd(server_fd);
    masterServer.getPorts().insert(std::make_pair(port, server_fd));
}

MasterServer Parsor::parse(std::string fileName)
{
    MasterServer masterServer;
    std::ifstream inputFile(fileName);

    if (!inputFile.is_open())
        exit(Ft::print("Error: failed to open @.", fileName, 1));

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
        else if (token == "root" && !directiveIsValid(currentServer.getRoot(), iss, token, SERVER, ""))
            exit(EXIT_FAILURE);
        else if (token == "server_name" && !directiveIsValid(currentServer.getServerName(), iss, token, SERVER, ""))
            exit(EXIT_FAILURE);
        else if (token == "listen")
        {
            // Récupérer le port d'écoute
            std::string listenPort;
            removeSemicolon(iss) >> listenPort;
            std::string additionalValue = checkNbParam(0, iss);
            //->//Check nombre de parametres
            if (!additionalValue.empty())
                exit(Ft::print("Server :Error :too many param for listen directive", NULL, EXIT_FAILURE));
            //->// Vérifier qu'il y a bien un port
            if (listenPort.empty())
                exit(Ft::print("Server :Error: missing param for listen directive", NULL, EXIT_FAILURE));
            //->// Vérifier que c'est bien un nombre
            if (!isNumeric(listenPort))
                exit(Ft::print("Server :Error: port '@' is not a valid port", listenPort, EXIT_FAILURE));
            //->// Vérifier si le port d'écoute existe déjà dans le current Server
            if (currentServer.getPorts().find(listenPort) != currentServer.getPorts().end())
                exit(Ft::print("Server :Error: port '@' is already set", listenPort, EXIT_FAILURE));
            for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
            {
                std::map<std::string, int> servPorts = it->getPorts();

                for (std::map<std::string, int>::iterator itPort = servPorts.begin(); itPort != servPorts.end(); itPort++)
                {
                    std::cout << itPort->first << " : " << itPort->second << std::endl;
                    if (servPorts.find(listenPort) != servPorts.end())
                        exit(Ft::print("Server :Error: port @ is already used by another server", listenPort, EXIT_FAILURE));
                }
            }
            setSocket(masterServer, currentServer, listenPort);
        }
        else if (token == "error_page")
        {
            // Récupérer la page d'erreur
            std::string errorPage;
            iss >> errorPage;

            std::string additionalValue = checkNbParam(1, iss);
            if (additionalValue.empty())
                exit(Ft::print("Server :Error: error_page directive has to have ONE param", NULL, EXIT_FAILURE));

            // Ajouter la paire à la map si errorPage est un nombre ATENTION VERIFIER LE PATH
            if (!isNumeric(errorPage))
                exit(Ft::print("Server :Error: error_page directive must be an integer", NULL, EXIT_FAILURE));

            if (!Ft::fileExists("./" + additionalValue))
                exit(Ft::print("Server :Error: invalid path for error_page @", errorPage, EXIT_FAILURE));

            for (std::map<int, std::string>::iterator itErrPages = currentServer.getErrorPages().begin(); itErrPages != currentServer.getErrorPages().end(); itErrPages++)
            {
                if (std::atoi(errorPage.c_str()) == itErrPages->first)
                    exit(Ft::print("Server :Error: error @ is already defined", errorPage, EXIT_FAILURE));
            }
            currentServer.getErrorPages()[std::atoi(errorPage.c_str())] = removeSemicolon(additionalValue);
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
                else if (directive == "client_max_body_size" && !directiveIsValid(currentLocation.max_body, locIss, directive, LOCATION, currentLocation.path))
                    exit(EXIT_FAILURE);
                else if (directive == "limit_except")
                {
                    std::string tmp;
                    int i = 0;
                    while (locIss >> tmp)
                    {
                        tmp = removeSemicolon(tmp);
                        if (tmp != "GET" && tmp != "POST" && tmp != "DELETE")
                        {
                            std::cout << tmp << " -> ";
                            std::cout << "invalid method\n";
                            exit(EXIT_FAILURE);
                        }
                        for (std::vector<std::string>::iterator it = currentLocation.limit_except.begin(); it != currentLocation.limit_except.end(); i++)
                        {
                            if (tmp == *it)
                            {
                                std::cout << "method already defined\n";
                                exit(EXIT_FAILURE);
                            }
                            currentLocation.limit_except.push_back(tmp);
                        }
                        i++;
                    }
                    if (i == 0)
                    {
                        std::cout << "missing arg\n";
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    // invalid directive
                }
            }
            currentServer.getLocations().push_back(currentLocation);
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
        if (it->getServerName().empty())
            it->setServerName("localhost");
    }

    masterServer.setServer(servers);

    return masterServer;
}
