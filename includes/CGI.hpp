#ifndef CGI_HPP
# define CGI_HPP

# include <unistd.h>
# include <map>
# include <iostream>
# include <sys/socket.h>
# include <sys/wait.h>

# include "Request.hpp"
# include "HttpResponse.hpp"
# include "MasterServer.hpp"

# define CGI_SCRIPT_PATH "/upload"
# define CGI_UPLOAD_SCRIPT_PATH "upload"

# define BUFFER_LENGTH 4096

# define CONTENT_TYPE "CONTENT_TYPE"
# define CONTENT_LENGTH "CONTENT_LENGTH"
# define HTTP_USER_AGENT "HTTP_USER_AGENT"
# define PATH_INFO "PATH_INFO"
# define REQUEST_METHOD "REQUEST_METHOD"
# define SCRIPT_FILENAME "SCRIPT_FILENAME"
# define SCRIPT_NAME "SCRIPT_NAME"
# define SERVER_PROTOCOL "SERVER_PROTOCOL"
# define QUERY_STRING "QUERY_STRING"

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