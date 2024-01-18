#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include "Client.hpp"

void handleCGIRequest(const Client &client);

#endif