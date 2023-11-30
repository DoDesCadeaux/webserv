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
    for (size_t i = 0; i < stop; i++){
        if (line[i] == '#')
            stop = i;
    }
    for (size_t i = stop - 1; i >= 0; --i){
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
int Parsor::parse(std::string configName)
{
    std::string line;
    std::ifstream file;
    std::vector<int> brackets;
    int itLine = 0;
    char last;

    file.open(configName);
    if (!file.is_open())
		return Ft::print("Error: failed to open @.", configName, 0);

    while (std::getline(file, line)){
        itLine++;
        last = findEnd(line);

        if (line[line.find_first_not_of(" \t", 0)] == '#' || last == '\0' || last == 4)
            continue;
        if (last == '{'){
            brackets.push_back(itLine);
            continue;
        }
        else if (last == '}'){
            if (brackets.empty())
                return Ft::print(ERR_CONF_EXTRA_BR, itLine, 0);
            brackets.pop_back();
            continue;
        }
        else if (last != ';'){
            return Ft::print(ERR_CONF_MISSING_PONCT, itLine, 0);
        }
    }
    if (!brackets.empty())
        return Ft::print(ERR_CONF_MISSING_BR, brackets.back(), 0);
 
    file.close();

    return 1;
}

