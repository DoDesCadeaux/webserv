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

std::string removeElement(std::string &input, std::string elementsToRemove)
{
    for (std::string::iterator it = elementsToRemove.begin(); it != elementsToRemove.end(); it++)
    {
        size_t pos = input.rfind(*it);
        if (pos != std::string::npos)
        {
            input = input.substr(0, pos);
        }
    }
    return input;
}

// Classe adaptateur pour utiliser removeElement avec l'opérateur >>
class RemoveElementAdapter
{
public:
    RemoveElementAdapter(std::istream &is, std::string elementsToRemove) : is_(is), elementsToRemove_(elementsToRemove) {}
    template <typename T>
    RemoveElementAdapter &operator>>(T &val)
    {
        is_ >> token_;
        val = removeElement(token_, elementsToRemove_);
        return *this;
    }

private:
    std::istream &is_;
    std::string token_;
    std::string elementsToRemove_;
};

RemoveElementAdapter removeElement(std::istream &is, std::string elementsToRemove)
{
    return RemoveElementAdapter(is, elementsToRemove);
}

static size_t findEnd(std::string line)
{
    size_t stop = line.size();
    for (size_t i = 0; i < stop; i++)
        if (line[i] == '#')
            stop = i;
    if (stop == 0)
        return stop;
    for (size_t i = stop - 1; i > 0; i--)
    {
        if (line[i] == 32 || line[i] == '\t')
            continue;
        return i;
    }
    return 0;
 }

void Parsor::parseIntegrity(std::string configName)
{
    std::string line;
    std::ifstream file;
    std::vector<int> brackets;
    int itLine = 0;
    size_t last;
    std::map<std::string, int> ports;

    file.open(configName);
    if (!file.is_open())
        exit(Ft::printErr("failed to open @.", configName, EXIT_FAILURE, "", &file, ports));

    while (std::getline(file, line))
    {
        itLine++;
        last = findEnd(line);
        
        if (line.empty() || line[last] == '#' || line[last] == 4 || line[last] == 32 || line[last] =='\t')
            continue;
		
        int count = 0;
        for (size_t i = 0; i <= last && i < line.size(); ++i)
        {
            if (line[i] == '{')
            {
                brackets.push_back(itLine);
                count++;
                continue;
            }
            else if (line[i] == '}')
            {
                if (brackets.empty())
                    exit(Ft::printErr(ERR_CONF_EXTRA_BR, NULL, EXIT_FAILURE, configName + ":" + Ft::to_string(itLine), &file, ports));
                brackets.pop_back();
                continue;
            }
        }
        if (line[last] != ';' && line[last] != '{' && line[last] != '}')
            exit(Ft::printErr(ERR_CONF_MISSING_PONCT, NULL, EXIT_FAILURE, configName + ":" + Ft::to_string(itLine), &file, ports));
        if (count > 1)
            exit((Ft::printErr("invalid syntax.", NULL, EXIT_FAILURE, configName + ":" + Ft::to_string(itLine), &file, ports)));
    }
    if (!brackets.empty())
        exit(Ft::printErr(ERR_CONF_MISSING_BR, NULL, EXIT_FAILURE, configName + ":" + Ft::to_string(brackets.back()), &file, ports));

    file.close();
}

bool isNumeric(const std::string &str)
{
    for (std::size_t i = 0; i < str.length(); ++i)
        if (!std::isdigit(str[i]))
            return false;
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
    return removeElement(additionalValue, ";");
}

std::string announceError()
{
	return fileConfig + ":" + Ft::to_string(i);
}

void alreadySet(std::string &toSave, std::string directive, std::ifstream *file, MasterServer masterServer)
{
    if (!toSave.empty())
        if ((directive == "server_name" && toSave != "localhost") || directive != "server_name")
            exit(Ft::printErr("@ is already set.", directive, EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
}

void goodArg(std::string &toSave, std::istringstream &iss, std::string directive, std::ifstream *file, MasterServer masterServer)
{
    removeElement(iss, ";") >> toSave;
    if (directive == "client_max_body_size")
        if (!isNumeric(toSave))
            exit(Ft::printErr("@ directive must be an integer.", directive, EXIT_FAILURE, announceError(), file, masterServer.getPorts()));

    std::string additionalValue = checkNbParam(0, iss);
    if (!additionalValue.empty())
        exit(Ft::printErr("too many param for @ directive.", directive, EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
    if (toSave.empty())
        exit(Ft::printErr("missing param for @ directive.", directive, EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
}

void directiveIsValid(std::string &toSave, std::istringstream &iss, std::string directive, std::ifstream *file, MasterServer masterServer)
{
    if (directive == "root" || directive == "server_name" || directive == "index" || directive == "autoindex" || directive == "auth" || directive == "upload" || directive == "client_max_body_size")
    {
        alreadySet(toSave, directive, file, masterServer);
        goodArg(toSave, iss, directive, file, masterServer);
    }
    if (directive == "autoindex" && toSave != "on" && toSave != "off")
        exit(Ft::printErr("autoindex directive must be set to on/off", NULL, EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
    if (directive == "root")
    {
        toSave = Ft::startsWith(toSave, "./") ? toSave : (Ft::startsWith(toSave, "/") ? "." + toSave : "./" + toSave);
        if (!Ft::fileExists(toSave))
            exit(Ft::printErr("@ is an invalid root.", toSave, EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
    }
}

std::map<std::string,std::string>    directiveCgiIsValid(std::istringstream &iss, std::ifstream *file, MasterServer masterserver)
{
    std::string tmp, key, value;
    std::map<std::string, std::string> cgi;
    int i = 0;
    (void)cgi;
    while (iss >> tmp)
    {
        if (!removeElement(tmp, ";").empty())
        {
            i++;
            if (i == 1)
                key = tmp;
            if (i == 2)
                value = tmp;
        }
    }
    if (i > 2)
        exit(Ft::printErr("cgi directive has to have TWO param.", NULL, EXIT_FAILURE, announceError(), file, masterserver.getPorts()));
    if (key.empty() || value.empty())
        exit(Ft::printErr("missing param for cgi directive.", NULL, EXIT_FAILURE, announceError(), file, masterserver.getPorts()));
    if (!Ft::startsWith(key, "."))
        exit(Ft::printErr("first param of cgi directive must be an extension.", NULL, EXIT_FAILURE, announceError(), file, masterserver.getPorts()));
    cgi.insert(std::make_pair(key, value));
    return cgi;
}

void setSocket(MasterServer &masterServer, Server &server, std::string port, std::ifstream *file)
{
    struct addrinfo hint, *servinfo;
    int server_fd;
    int yes = 1;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port.c_str(), &hint, &servinfo) != 0)
        exit(Ft::printErr("@.", strerror(errno), EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
    server_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (server_fd == -1)
        exit(Ft::printErr("@.", strerror(errno), EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        exit(Ft::printErr("@.", strerror(errno), EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
    if (bind(server_fd, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        exit(Ft::printErr("@ with port " + port, strerror(errno), EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
    if (listen(server_fd, 1000) < 0)
        exit(Ft::printErr("@.", strerror(errno), EXIT_FAILURE, announceError(), file, masterServer.getPorts()));
    freeaddrinfo(servinfo);
    server.getPorts().insert(std::make_pair(port, server_fd));
    masterServer.addFd(server_fd);
    masterServer.getPorts().insert(std::make_pair(port, server_fd));
    std::cout << "Listening on port " << port << std::endl;
}

std::string getPath(std::istringstream &iss, std::ifstream *inputFile, MasterServer masterServer)
{
    std::vector<std::string> vectorLoc;
    std::string tmp;
    while (iss >> tmp)
    {
        if (!removeElement(tmp, "{").empty())
            vectorLoc.push_back(tmp);
    }
    std::vector<std::string>::iterator first = vectorLoc.begin();
    if (Ft::startsWith(*first, "~") || Ft::startsWith(*first, "@") || Ft::startsWith(*first, "=") || Ft::startsWith(*first, "*") )
        exit(Ft::printErr("location wilcard not accepted.", NULL, EXIT_FAILURE, announceError(), inputFile, masterServer.getPorts()));
    if (!Ft::startsWith(*first, "/"))
        exit(Ft::printErr("location path must start with '/'.", NULL, EXIT_FAILURE, announceError(), inputFile, masterServer.getPorts()));
    if (vectorLoc.size() != 1)
        exit(Ft::printErr("location directive has to have ONE param.", NULL, EXIT_FAILURE, announceError(), inputFile, masterServer.getPorts()));
    return *first;
}

MasterServer Parsor::parse(std::string fileName)
{
    MasterServer		        masterServer;
    std::ifstream		        inputFile(fileName);
	std::string 		        line;
    Location 			        currentLocation;
    Server 				        currentServer;
    std::vector<Server>         servers;
    std::map<std::string, int>  ports;

    if (!inputFile.is_open())
        exit(Ft::printErr("failed to open @.", &fileName, EXIT_FAILURE, NULL, NULL, ports));
	fileConfig = fileName;

    while (std::getline(inputFile, line))
    {
        std::istringstream iss(line);
        std::string token;
        removeElement(iss, ";{") >> token;
        if (token == "server")
            currentServer = Server();
        else if (token == "root")
            directiveIsValid(currentServer.getRoot(), iss, token, &inputFile, masterServer);
        else if (token == "server_name")
            directiveIsValid(currentServer.getServerName(), iss, token, &inputFile, masterServer);
        else if (token == "listen")
        {
            std::string listenPort;
            removeElement(iss, ";") >> listenPort;
            std::string additionalValue = checkNbParam(0, iss);
            if (!additionalValue.empty())
                exit(Ft::printErr("too many param for listen directive", NULL, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            if (listenPort.empty())
                exit(Ft::printErr("missing param for listen directive", NULL, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            if (!isNumeric(listenPort))
                exit(Ft::printErr("port '@' is not a valid port", listenPort, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            if (currentServer.getPorts().find(listenPort) != currentServer.getPorts().end())
                exit(Ft::printErr("port '@' is already set", listenPort, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
            {
                std::map<std::string, int> servPorts = it->getPorts();
                for (std::map<std::string, int>::iterator itPort = servPorts.begin(); itPort != servPorts.end(); itPort++)
                    if (servPorts.find(listenPort) != servPorts.end())
                        exit(Ft::printErr("port @ is already used by another server", listenPort, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            }
            setSocket(masterServer, currentServer, listenPort, &inputFile);
        }
        else if (token == "error_page")
        {
            std::string errorPage;
            iss >> errorPage;

            std::string additionalValue = checkNbParam(1, iss);
            if (additionalValue.empty())
                exit(Ft::printErr("error_page directive has to have ONE param", NULL, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            if (!isNumeric(errorPage))
                exit(Ft::printErr("error_page directive must be an integer", NULL, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            if (!Ft::fileExists("./" + additionalValue))
                exit(Ft::printErr("invalid path for error_page @", errorPage, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            for (std::map<int, std::string>::iterator itErrPages = currentServer.getErrorPages().begin(); itErrPages != currentServer.getErrorPages().end(); itErrPages++)
                if (std::atoi(errorPage.c_str()) == itErrPages->first)
                    exit(Ft::printErr("error @ is already defined", errorPage, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            currentServer.getErrorPages()[std::atoi(errorPage.c_str())] = removeElement(additionalValue, ";");
        }
        else if (token == "location")
        {
            currentLocation = Location();
            currentLocation.path = getPath(iss, &inputFile, masterServer);

            std::string directive;
            while (std::getline(inputFile, line))
            {
                std::istringstream locIss(line);
                removeElement(locIss, ";") >> directive;
				++i;
                if (directive == "}")
                    break;
                else if (directive == "root")
                    directiveIsValid(currentLocation.root, locIss, directive, &inputFile, masterServer);
                else if (directive == "index")
                    directiveIsValid(currentLocation.index, locIss, directive, &inputFile, masterServer);
                else if (directive == "autoindex")
                    directiveIsValid(currentLocation.autoindex, locIss, directive, &inputFile, masterServer);
                else if (directive == "cgi")
                    currentLocation.cgi = directiveCgiIsValid(locIss, &inputFile, masterServer);
                else if (directive == "auth")
                    directiveIsValid(currentLocation.auth, locIss, directive, &inputFile, masterServer);
                else if (directive == "upload")
                    directiveIsValid(currentLocation.upload, locIss, directive, &inputFile, masterServer);
                else if (directive == "client_max_body_size")
                    directiveIsValid(currentLocation.max_body, locIss, directive, &inputFile, masterServer);
                else if (directive == "limit_except")
                {
                    std::string tmp;
					int param = 0;
                    while (locIss >> tmp)
                    {
                        tmp = removeElement(tmp, ";");
                        if (tmp != "GET" && tmp != "POST" && tmp != "DELETE")
                            exit(Ft::printErr("invalid method for @ directive", directive, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
                        if (currentLocation.limit_except.empty())
                            currentLocation.limit_except.push_back(tmp);
                        else
                        {
                            for (std::vector<std::string>::iterator it = currentLocation.limit_except.begin(); it != currentLocation.limit_except.end(); it++)
                                if (tmp == *it)
                					exit(Ft::printErr("method @ is already defined", tmp, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
                            currentLocation.limit_except.push_back(tmp);
                        }
						param++;
                    }
					if (param == 0)
						exit(Ft::printErr("missing param for @ directive", directive, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
                }
                else
                    if (!directive.empty() && !Ft::startsWith(directive, "#"))
                		exit(Ft::printErr("@ is an invalid directive", directive, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
            }
            currentServer.getLocations().push_back(currentLocation);
        }
        else if (token == "}")
            servers.push_back(currentServer);
        else
            if (!token.empty() && !Ft::startsWith(token, "#"))
                exit(Ft::printErr("@ is an invalid directive", token, EXIT_FAILURE, announceError(), &inputFile, masterServer.getPorts()));
		i++;
    }

    if (servers.empty())
        exit(Ft::printErr("missing server bloc.", NULL, EXIT_FAILURE, NULL, &inputFile, masterServer.getPorts()));

    masterServer.setServer(servers);
    inputFile.close();

    return masterServer;
}
