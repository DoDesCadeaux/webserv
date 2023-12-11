/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamartin <pamartin@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/11 13:54:30 by pamartin          #+#    #+#             */
/*   Updated: 2023/12/11 13:54:31 by pamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <sys/socket.h>


class Client{
    private:
        int						            _fd;
		struct sockaddr_storage         	_addr;
		bool					            _connect;
		int                                 _fdport;
    public:
        Client(int fd, struct sockaddr_storage addr, bool connect, int fdport);
        int getFdPort() const;
        int getFd() const;

};
#endif
