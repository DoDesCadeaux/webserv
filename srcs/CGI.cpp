#include "CGI.hpp"

//Destructor
Cgi::~Cgi() {
	this->_envVariables.clear();
	//DEBUG bien free !
}

//Default constructor
Cgi::Cgi() {}

//Constructor
Cgi::Cgi(const Request req, int socketFd) {
	this->_req = req;
	this->_socketFd = socketFd;
	this->_exec = this->_req.getUri();
	Cgi::initEnv();
}

//Copy constructor
Cgi::Cgi(Cgi const &src) {
	*this = src;
}

//Override operator =
Cgi &Cgi::operator=(Cgi const &rhs) {
	if (this != &rhs) {
		this->_envVariables = rhs.getEnvVariables();
		this->_exec = rhs.getExec();
		this->_req = rhs.getRequest();
		this->_socketFd = rhs.getSocketFd();
	}
	return (*this);
}

Request	 Cgi::getRequest(void) const {
	return (this->_req);
}

std::string	Cgi::getExec(void) const {
	return (this->_exec);
}

std::map<std::string, std::string>	Cgi::getEnvVariables(void) const {
	return (this->_envVariables);
}

int	Cgi::getSocketFd(void) const {
	return (this->_socketFd);
}

/*
** Create the association between a field and it's value for the CGI environment
**	variables.
** @param :	std::string leftString the name of the variable
**			std::string 
*/
std::string	Cgi::createEnvString(std::string leftString, std::string righString) {
	return (leftString + "=" + righString);
}

/*
** Initialise all the environment variables for the CGI
*/
void    Cgi::initEnv() {
//exemple de env
	// env.variables["PATH_INFO"] = path;
    // env.variables["ROOT"] = "/path/to/your/root";
    // env.variables["REQUEST_METHOD"] = "GET";
    // env.variables["CONTENT_TYPE"] = "text/html";
    // env.variables["CONTENT_LENGTH"] = "0";
    // env.variables["QUERY_STRING"] = "param1=value1&param2=value2";
    // env.variables["REDIRECT_STATUS"] = "200";
//Protocol de Dorian = Method (GET/POST)
	
	// this->_envVariables[HTTP_USER_AGENT] = createEnvString(HTTP_USER_AGENT, this->_req.getHeaderFields()[USER_AGENT_FIELD]);
	// this->_envVariables[CONTENT_TYPE] = createEnvString(CONTENT_TYPE, this->_req.getHeaderFields()[CONTENT_TYPE_FIELD]);
	// this->_envVariables[QUERY_STRING] = createEnvString(QUERY_STRING, this->_req.getQueryString());
	this->_envVariables[CONTENT_LENGTH] = createEnvString(CONTENT_LENGTH, std::to_string(this->_req.getBodyPayload().size()));
	this->_envVariables[CONTENT_TYPE] = createEnvString(CONTENT_TYPE, _req.getBodyPayload());			//bodyPayload est toujours vide à ce stade là...?
	this->_envVariables[PATH_INFO] = createEnvString(PATH_INFO, this->_req.getUri());
	this->_envVariables[REQUEST_METHOD] = createEnvString(REQUEST_METHOD, this->_req.getProtocol());	//Protocol de Dorian = Method (GET/POST)
	this->_envVariables[SCRIPT_FILENAME] = createEnvString(SCRIPT_FILENAME, this->_req.getUri());
	this->_envVariables[SCRIPT_NAME] = createEnvString(SCRIPT_NAME, this->_req.getUri());
	this->_envVariables[SERVER_PROTOCOL] = createEnvString(SERVER_PROTOCOL, this->_req.getProtocol());

	//print for debug
	std::cout << std::endl;
	std::map<std::string, std::string>::iterator it;
    for (it = _envVariables.begin(); it != _envVariables.end(); ++it) {
        std::cout << it->second << std::endl;
    }
	std::cout << std::endl;
	///
}

// bool    isCgiRequest(const Request &request) {
//     if (request.getUri() == CGI_SCRIPT_PATH) {
// 		std::cout << "It's a CGI request (1)\n";
//         return (true);
//     }
//     else if (request.getUri() == CGI_UPLOAD_SCRIPT_PATH /*&& request.getProtocol() == "POST"*/) {
// 		std::cout << "It's a CGI request (2)\n";
//         return (true);
//     }
//     std::cout << "It's not a CGI request\r\n";
//     return (false);
// }

int     Cgi::handleCGIRequest() {
    int		in_pipe[2];
	int		out_pipe[2];
	pid_t   pid = 0;
	int 	status = 0;

	char *env[] = {&this->_envVariables[CONTENT_TYPE][0],
					&this->_envVariables[CONTENT_LENGTH][0],
					&this->_envVariables[HTTP_USER_AGENT][0],
					&this->_envVariables[PATH_INFO][0],
					&this->_envVariables[REQUEST_METHOD][0],
					&this->_envVariables[SCRIPT_FILENAME][0],
					&this->_envVariables[SCRIPT_NAME][0],
					&this->_envVariables[SERVER_PROTOCOL][0],
					&this->_envVariables[QUERY_STRING][0],
					NULL};

	if (pipe(in_pipe) == -1) {
		perror("pipe");
		return(-1);
	}
	if (pipe(out_pipe) == -1) {
		close(in_pipe[0]);
		close(in_pipe[1]);
		perror("pipe");
		return(-1);
	}
    pid = fork();
    if (pid == -1) {
		perror("pipe");
		return(-1);
	}
    // Child process
	if (!pid) {
        close(in_pipe[1]);
        close(out_pipe[0]);

		if (dup2(in_pipe[0], STDIN_FILENO) == -1) {
			perror("dup2");
			exit(EXIT_FAILURE);
		}
		close(in_pipe[0]);
        if (dup2(out_pipe[1], STDOUT_FILENO) == -1) {
			perror("dup2");
			exit(EXIT_FAILURE);
		}
        close(out_pipe[1]);
		
		execve(this->_exec.c_str(), NULL, env);
		perror("execve");
		exit(EXIT_FAILURE);
    }
	// Parent process
    else {
        int				retPid = 0;
		int				ret = 0;
		char			buf[BUFFER_LENGTH];
		std::string 	body;
		std::string		toRet;

		close(in_pipe[0]);
		close(out_pipe[1]);

		body = this->_req.getBodyPayload();
		status = write(in_pipe[1], body.c_str(), body.size());
		if (status < 0) {
			perror("write");
			return(-1);
		}
		close(in_pipe[1]);
		
		while((ret = read(out_pipe[0], buf, BUFFER_LENGTH))) {
			if (ret < 0) {
				perror("read");
				return(-1);
			}
			toRet = buf;
			status = send(this->_socketFd, toRet.c_str(), toRet.size(), 0);
		}
		close(out_pipe[0]);
		waitpid(pid, &retPid, 0);
		if (WIFEXITED(retPid) ) {
			HttpResponse response;
			response.setErrorResponse(500, "Something went wrong with the child process");
			
			std::string toSend = response.getResponse();
			status = send(this->_socketFd, toSend.c_str(), toSend.size(), 0);
		}	
    }
    return (status);
}
