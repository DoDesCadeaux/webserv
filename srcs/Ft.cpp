/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Ft.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamartin <pamartin@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/28 10:27:44 by pamartin          #+#    #+#             */
/*   Updated: 2023/11/28 10:27:46 by pamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Ft.hpp"

void Ft::printSet(fd_set set, std::string name) {
    std::cout << name << " : { ";

    for (int i = 0; i < FD_SETSIZE; ++i) {
        if (FD_ISSET(i, &set)) {
            std::cout << i << " ";
        }
    }
    std::cout << "}" << std::endl;
}

void Ft::printClient(std::map<int, Client *> clients) {
	std::cout << "clients : [ ";
	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
		std::cout << it->first << " ";
	}
	std::cout << "]" << std::endl;
}
