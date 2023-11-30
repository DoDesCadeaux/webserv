/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pamartin <pamartin@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/28 09:50:41 by pamartin          #+#    #+#             */
/*   Updated: 2023/11/28 09:50:44 by pamartin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parsor.hpp"
#include "Server.hpp"
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
// #include <sys/socket.h>
// #include <sys/types.h>

int main(int argc, char **argv){
    if (argc > 2)
        return Ft::print("Error: bad arg", NULL, EXIT_FAILURE);

    std::string fileConf = argv[1] ? argv[1] : "file.conf";
    if (!Parsor::parse(fileConf))
        return (EXIT_FAILURE);

    Server server;
    server.setSocket();

    // int status, sockFD;
    // struct addrinfo hints;
    // struct addrinfo *servinfo, *rp;
    
    // //Pour etre sur que hint est bien vide
    // memset(&hints, 0, sizeof hints);
    // //Famille d'addressage renvoyée -> gere ipv4 et 6
    // hints.ai_family = AF_UNSPEC;
    // //Type de socket
    // hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE;

    // status = getaddrinfo(NULL, "4242", &hints, &servinfo);

    // if (status != 0){
    //     printf("getaddrinfo: %s", gai_strerror(status));
    // }

    // for (rp = servinfo ; rp != NULL; rp = rp->ai_next){
    //     sockFD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    //     if (sockFD == -1) continue;
    //     fcntl(sockFD, F_SETFL, O_NONBLOCK);
    //     if (bind(sockFD, rp->ai_addr, rp->ai_addrlen) == 0){
    //         printf("Bind on port 4242 with %d", sockFD);
    //         break;
    //     }
    //     close(sockFD);
    // }
    // if (rp == NULL){
    //     printf("could not bind()");
    //     return EXIT_FAILURE;
    // }
    // freeaddrinfo(servinfo);

    // while (1){
    // if (connect(sockFD, rp->ai_addr, rp->ai_addrlen) != -1)
    //     printf("connect ok");
    // }
    return 0;
}
