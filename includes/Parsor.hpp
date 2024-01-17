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
#define PARSOR_HPP

#include <iostream>
#include <fstream>
#include "Ft.hpp"
#include <vector>
#include "Server.hpp"
#include "MasterServer.hpp"
#include <sys/stat.h>

#define ROOT "root"
#define SERV_NAME "server_name"
#define LISTEN "listen"
#define ERR_PAGE "error_page"
#define LOCATION "location"
#define SERVER "server"

class MasterServer;
class Parsor
{
public:
    static void parseIntegrity(std::string configName);
    static MasterServer parse(std::string configName);
};

#endif
