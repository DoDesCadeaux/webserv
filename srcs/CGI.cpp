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

//Override = operator
Cgi &Cgi::operator=(Cgi const &rhs) {
	if (this != &rhs) {
		this->_envVariables = rhs.getEnvVariables();
		this->_exec = rhs.getExec();
		this->_req = rhs.getRequest();
		this->_socketFd = rhs.getSocketFd();
	}
	return (*this);
}

/*
** Getter fot the request field.
** @return : request : request 
*/
Request	 Cgi::getRequest(void) const {
	return (this->_req);
}

/*
** Getter for the executable fields.
** @return : executable : std::string 
*/
std::string	Cgi::getExec(void) const {
	return (this->_exec);
}

/*
** Getter for the environement variables fields.
** @return : envVariables : std::map<std::string, std::string> 
*/
std::map<std::string, std::string>	Cgi::getEnvVariables(void) const {
	return (this->_envVariables);
}

/*
** Getter for the socketFd fields.
** @return : socketFd : int 
*/
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
** Initialise all the environment variables for the cgi.
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

	std::cout << "Init ENV\n";
    this->_envVariables[URI] = createEnvString(URI, this->_req.getUri());
    this->_envVariables[METHOD] = createEnvString(METHOD, this->_req.getProtocol());
	this->_envVariables[BODY] = createEnvString(BODY, this->_req.getBodyPayload());
	this->_envVariables[BODY_LENGTH] = createEnvString(BODY_LENGTH, std::to_string(this->_req.getBodyPayload().size()));
//this->_envVariables[FORMAT] = createEnvString(FORMAT, req.getFormat());
//this->_envVariables[HEADER] = createEnvString(HEADER, this->_req.getHeader(???));
//this->_envVariables[LINE] = createEnvString(LINE, this->_req.getLineRequest());
	
	//print for debug
	std::map<std::string, std::string>::iterator it;
    for (it = _envVariables.end(); it != _envVariables.begin(); --it) {
        std::cout << it->second << std::endl;
    }
	
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

//Finish when initEnv() is debugged
	char *env[] = {&this->_envVariables[URI][0],
					&this->_envVariables[METHOD][0],
					&this->_envVariables[BODY_LENGTH][0],
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
			if (status < 0) {
				perror("send");
				return(-1);
			}
		}
		close(out_pipe[0]);
		waitpid(pid, &retPid, 0);
		if (WIFEXITED(retPid) ) {
			HttpResponse response;
			response.setErrorResponse(500, "Something went wrong with the child process");
			
			std::string toSend = response.getResponse();
			status = send(this->_socketFd, toSend.c_str(), toSend.size(), 0);
			if (status < 0) {
				perror("send");
				return(-1);
			}
		}	
    }
    return (status);
}
