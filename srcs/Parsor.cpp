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
int i = 1;
std::string fileConfig;

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
    int stop = line.size();
    for (int i = 0; i < stop; i++)
    {
        if (line[i] == '#')
            stop = i;
    }
    for (int i = stop - 1; i >= 0; --i)
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
        return Ft::printErr("failed to open @.", configName, 0, "");

    while (std::getline(file, line))
    {
        itLine++;
        last = findEnd(line);
        if (line.empty() || last == '\0' || last == 4)
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
                return Ft::printErr(ERR_CONF_EXTRA_BR, NULL, 0, configName + ":" + Ft::to_string(itLine));
            brackets.pop_back();
            continue;
        }
        else if (last != ';')
            return Ft::printErr(ERR_CONF_MISSING_PONCT, NULL, 0, configName + ":" + Ft::to_string(itLine));
    }
    if (!brackets.empty())
        return Ft::printErr(ERR_CONF_MISSING_BR, NULL, 0, configName + ":" + Ft::to_string(brackets.back()));

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

std::string announceError()
{
	return fileConfig + ":" + Ft::to_string(i);
}

void alreadySet(std::string &toSave, std::string directive)
{
    if (!toSave.empty())
        if ((directive == "server_name" && toSave != "localhost") || directive != "server_name")
            exit(Ft::printErr("@ is already set.", directive, EXIT_FAILURE, announceError()));
}

void goodArg(std::string &toSave, std::istringstream &iss, std::string directive)
{
    removeSemicolon(iss) >> toSave;
    if (directive == "client_max_body_size")
        if (!isNumeric(toSave))
            exit(Ft::printErr("@ directive must be an integer.", directive, EXIT_FAILURE, announceError()));

    std::string additionalValue = checkNbParam(0, iss);
    if (!additionalValue.empty())
        exit(Ft::printErr("too many param for @ directive.", directive, EXIT_FAILURE, announceError()));
    if (toSave.empty())
        exit(Ft::printErr("missing param for @ directive.", directive, EXIT_FAILURE, announceError()));
}

void directiveIsValid(std::string &toSave, std::istringstream &iss, std::string directive)
{
    if (directive == "root" || directive == "server_name" || directive == "index" || directive == "autoindex" || directive == "auth" || directive == "upload" || directive == "client_max_body_size")
    {
        alreadySet(toSave, directive);
        goodArg(toSave, iss, directive);
    }
    if (directive == "autoindex" && toSave != "on" && toSave != "off")
        exit(Ft::printErr("autoindex directive must be set to on/off", NULL, EXIT_FAILURE, announceError()));
    if (directive == "root")
    {
        toSave = Ft::startsWith(toSave, "./") ? toSave : (Ft::startsWith(toSave, "/") ? "." + toSave : "./" + toSave);
        if (!Ft::fileExists(toSave))
            exit(Ft::printErr("@ is an invalid root.", toSave, EXIT_FAILURE, announceError()));
    }
}

void setSocket(MasterServer &masterServer, Server &server, std::string port)
{
    struct addrinfo hint, *servinfo;
    int server_fd;
    int yes = 1;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port.c_str(), &hint, &servinfo) != 0)
    {
        perror("Address Info");
        exit(EXIT_FAILURE);
    }
    server_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (server_fd == -1)
    {
        perror("Socket");
        exit(EXIT_FAILURE);
    }
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        perror("Setsockopt");
        exit(EXIT_FAILURE);
    }
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
    freeaddrinfo(servinfo);
    server.getPorts().insert(std::make_pair(port, server_fd));
    masterServer.addFd(server_fd);
    masterServer.getPorts().insert(std::make_pair(port, server_fd));
}


MasterServer Parsor::parse(std::string fileName)
{
    MasterServer		masterServer;
    std::ifstream		inputFile(fileName);
	std::string 		line;
    Location 			currentLocation;
    Server 				currentServer;
    std::vector<Server> servers;

    if (!inputFile.is_open())
        exit(Ft::printErr("failed to open @.", fileName, EXIT_FAILURE, NULL));
	fileConfig = fileName;

    while (std::getline(inputFile, line))
    {
        std::istringstream iss(line);
        std::string token;
        removeSemicolon(iss) >> token;

        if (token == "server{")
            currentServer = Server();
        else if (token == "root")
            directiveIsValid(currentServer.getRoot(), iss, token);
        else if (token == "server_name")
            directiveIsValid(currentServer.getServerName(), iss, token);
        else if (token == "listen")
        {
            std::string listenPort;
            removeSemicolon(iss) >> listenPort;
            std::string additionalValue = checkNbParam(0, iss);
            if (!additionalValue.empty())
                exit(Ft::printErr("too many param for listen directive", NULL, EXIT_FAILURE, announceError()));
            if (listenPort.empty())
                exit(Ft::printErr("missing param for listen directive", NULL, EXIT_FAILURE, announceError()));
            if (!isNumeric(listenPort))
                exit(Ft::printErr("port '@' is not a valid port", listenPort, EXIT_FAILURE, announceError()));
            if (currentServer.getPorts().find(listenPort) != currentServer.getPorts().end())
                exit(Ft::printErr("port '@' is already set", listenPort, EXIT_FAILURE, announceError()));
            for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
            {
                std::map<std::string, int> servPorts = it->getPorts();
                for (std::map<std::string, int>::iterator itPort = servPorts.begin(); itPort != servPorts.end(); itPort++)
                    if (servPorts.find(listenPort) != servPorts.end())
                        exit(Ft::printErr("port @ is already used by another server", listenPort, EXIT_FAILURE, announceError()));
            }
            setSocket(masterServer, currentServer, listenPort);
        }
        else if (token == "error_page")
        {
            std::string errorPage;
            iss >> errorPage;

            std::string additionalValue = checkNbParam(1, iss);
            if (additionalValue.empty())
                exit(Ft::printErr("error_page directive has to have ONE param", NULL, EXIT_FAILURE, announceError()));
            if (!isNumeric(errorPage))
                exit(Ft::printErr("error_page directive must be an integer", NULL, EXIT_FAILURE, announceError()));
            if (!Ft::fileExists("./" + additionalValue))
                exit(Ft::printErr("invalid path for error_page @", errorPage, EXIT_FAILURE, announceError()));
            for (std::map<int, std::string>::iterator itErrPages = currentServer.getErrorPages().begin(); itErrPages != currentServer.getErrorPages().end(); itErrPages++)
                if (std::atoi(errorPage.c_str()) == itErrPages->first)
                    exit(Ft::printErr("error @ is already defined", errorPage, EXIT_FAILURE, announceError()));
            currentServer.getErrorPages()[std::atoi(errorPage.c_str())] = removeSemicolon(additionalValue);
        }
        else if (token == "location")
        {
            currentLocation = Location();
            iss >> currentLocation.path;
            std::string directive;
            while (std::getline(inputFile, line))
            {
                std::istringstream locIss(line);
                removeSemicolon(locIss) >> directive;
				++i;
                if (directive == "}")
                    break;
                else if (directive == "root")
                    directiveIsValid(currentLocation.root, locIss, directive);
                else if (directive == "index")
                    directiveIsValid(currentLocation.index, locIss, directive);
                else if (directive == "autoindex")
                    directiveIsValid(currentLocation.autoindex, locIss, directive);
                else if (directive == "cgi")
                {
                    std::string param;
                    removeSemicolon(locIss) >> param;
                    std::string additionalValue = checkNbParam(1, locIss);
					//Pas sure
                    if (additionalValue.empty())
                		exit(Ft::printErr("cgi directive has to have ONE param", NULL, EXIT_FAILURE, announceError()));
                    currentLocation.cgi[param] = removeSemicolon(additionalValue);
                }
                else if (directive == "auth")
                    directiveIsValid(currentLocation.auth, locIss, directive);
                else if (directive == "upload")
                    directiveIsValid(currentLocation.upload, locIss, directive);
                else if (directive == "client_max_body_size")
                    directiveIsValid(currentLocation.max_body, locIss, directive);
                else if (directive == "limit_except")
                {
                    std::string tmp;
					int param = 0;
                    while (locIss >> tmp)
                    {
                        tmp = removeSemicolon(tmp);
                        if (tmp != "GET" && tmp != "POST" && tmp != "DELETE")
                            exit(Ft::printErr("invalid method for @ directive", directive, EXIT_FAILURE, announceError()));
                        if (currentLocation.limit_except.empty())
                            currentLocation.limit_except.push_back(tmp);
                        else
                        {
                            for (std::vector<std::string>::iterator it = currentLocation.limit_except.begin(); it != currentLocation.limit_except.end(); it++)
                                if (tmp == *it)
                					exit(Ft::printErr("method @ is already defined", tmp, EXIT_FAILURE, announceError()));
                            currentLocation.limit_except.push_back(tmp);
                        }
						param++;
                    }
					if (param == 0)
						exit(Ft::printErr("missing param for @ directive", directive, EXIT_FAILURE, announceError()));
                }
                else
                    if (!directive.empty() && !Ft::startsWith(directive, "#"))
                		exit(Ft::printErr("@ is an invalid directive", directive, EXIT_FAILURE, announceError()));
            }
            currentServer.getLocations().push_back(currentLocation);
        }
        else if (token == "}")
            servers.push_back(currentServer);
        else
            if (!token.empty() && !Ft::startsWith(token, "#"))
                exit(Ft::printErr("@ is an invalid directive", token, EXIT_FAILURE, announceError()));
		i++;
    }

    if (servers.empty())
        exit(Ft::printErr("missing server bloc.", NULL, EXIT_FAILURE, NULL));

    masterServer.setServer(servers);
    inputFile.close();

    return masterServer;
}
