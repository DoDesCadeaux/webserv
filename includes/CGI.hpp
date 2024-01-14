#ifndef CGI_HPP
# define CGI_HPP

# include <unistd.h>
# include <map>
# include <iostream>
# include "Request.hpp"
# include "HttpResponse.hpp"
# include <sys/socket.h>
# include <sys/wait.h>

# define CGI_SCRIPT_PATH ""
# define CGI_UPLOAD_SCRIPT_PATH ""

# define BUFFER_LENGTH 4096

# define URI "URI"
# define METHOD "METHOD"
# define BODY_LENGTH "BODY_LENGTH"
# define FORMAT "FORMAT"
# define BODY "BODY"
# define HEADER "HEADER"

class Request;

class Cgi {
    private:
        Request									_req;
        std::string								_exec;
        int										_socketFd;
        std::map<std::string, std::string>		_envVariables;
		void									initEnv();
		Cgi();

	public:
		Cgi(const Request req, int socketFd);
		Cgi(Cgi const &src);
		Cgi &operator=(Cgi const &rhs);
		~Cgi();

		Request									getRequest() const;
		std::string								getExec() const;
		std::map<std::string, std::string>		getEnvVariables() const;
		int										getSocketFd() const;

		std::string								createEnvString(std::string leftString, std::string rightStrinng);
//		bool									isCGIRequest(const Request &request);
		int    									handleCGIRequest();
};

#endif