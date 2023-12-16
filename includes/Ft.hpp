#ifndef FT_HPP
# define FT_HPP

#include <iostream>
#include "../includes/Server.hpp"
#include <map>

#define ERR_CONF_EXTRA_BR "Error config file: extra bracket line @."
#define ERR_CONF_MISSING_BR "Error config file: missing closing bracket to match this '{' in the line @."
#define ERR_CONF_MISSING_PONCT "Error config file: missing ponctuation line @."

class Ft{
	public:
		template <class T>
		static int print(std::string msg, T elem, int err);
		static void printSet(const fd_set &set, const std::string &name);
		static void printClient(const std::map<int, Client *> &clients);
		static bool fileExists(const std::string& filePath);
};

//QUID de la valeur de retour d'erreur
/**
 * @brief ft_printf simplifié avec l'utilisation de '@'
 *
 * @param msg Texte que l'on veux écrire
 * @param elem Template element qui serra écrit a la place du @ dans le message
 *
 */
template <class T>
int Ft::print(std::string msg, T elem, int err){
	if (!msg.empty()){
		for (size_t i = 0; i < msg.size(); i++){
			if (msg[i] != '@')
				std::cout << msg[i];
			else
				std::cout << elem;
		}
	}
	else
		std::cout << elem;
	std::cout << std::endl;
	return err;
}


#endif
