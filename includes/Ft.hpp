#ifndef FT_HPP
#define FT_HPP

#include <iostream>
#include "Server.hpp"
#include "Client.hpp"
#include <map>
#include <sstream>
#include <unistd.h>

#define NOCOL "\e[39m"
#define RED "\e[91m"
#define ERR_CONF_EXTRA_BR "extra bracket."
#define ERR_CONF_MISSING_BR "missing closing bracket to match this '{'."
#define ERR_CONF_MISSING_PONCT "missing ponctuation."
class Server;
class Client;

class Ft
{
public:
	template <class T>
	static int printErr(std::string msg, T elem, int err, std::string loc, std::ifstream *file, std::map<std::string, int> ports);
	static void printSet(const fd_set &set, const std::string &name);
	static void printClient(const std::map<int, Client *> &clients);
	static bool fileExists(const std::string &filePath);
	static bool endsWith(const std::string &str, const std::string &suffix);
	static void printLogs(Server &serv, Client &client, std::string type);
	static bool startsWith(const std::string &str, const std::string &prefix);
	static std::string to_string(int nb);
};

// QUID de la valeur de retour d'erreur
/**
 * @brief ft_printf simplifié avec l'utilisation de '@'
 *
 * @param msg Texte que l'on veux écrire
 * @param elem Template element qui serra écrit a la place du @ dans le message
 *
 */
template <class T>
int Ft::printErr(std::string msg, T elem, int err, std::string loc, std::ifstream *file, std::map<std::string, int> ports)
{
	if (!loc.empty())
		std::cout << loc << " ";
	std::cout << RED << "error: " << NOCOL;
	if (!msg.empty())
	{
		for (size_t i = 0; i < msg.size(); i++)
		{
			if (msg[i] != '@')
			{

				std::cout << msg[i];
			}
			else
				std::cout << elem;
		}
	}
	else
		std::cout << elem;
	std::cout << std::endl;
	if (file)
		file->close();
	for (std::map<std::string, int>::iterator it = ports.begin(); it != ports.end(); it++)
	{
		close(it->second);
		std::cout << "Stop listening on port " << it->first << std::endl;
	}
	return err;
}

#endif
