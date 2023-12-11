/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamartin <pamartin@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/11 14:46:47 by pamartin          #+#    #+#             */
/*   Updated: 2023/12/11 14:46:48 by pamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Client::Client(int fd, struct sockaddr_storage addr, bool connect) : _fd(fd), _addr(addr), _connect(connect){
    _fd = fd;
    _addr = addr;
    _connect = connect;
}
