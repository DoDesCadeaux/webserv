/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsor.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamartin <pamartin@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/28 14:01:57 by pamartin          #+#    #+#             */
/*   Updated: 2023/11/28 14:01:58 by pamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSOR_HPP
# define PARSOR_HPP

#include <iostream>
#include <fstream>
#include "Ft.hpp"
#include <vector>

class Parsor{
public:
    static int parseIntegrity(std::string configName);
    static int parse(std::string configName);

};

#endif
