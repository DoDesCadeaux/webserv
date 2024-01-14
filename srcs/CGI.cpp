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
	std::cout << "CONSTRUCTOR called\n";
	this->_req = req;
	this->_socketFd = socketFd;
	this->_exec = this->_req.getUri();
	Cgi::initEnv(req);
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
void    Cgi::initEnv(Request req) {
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
    this->_envVariables[URI] = createEnvString(URI, req.getUri());
    this->_envVariables[METHOD] = createEnvString(METHOD, req.getProtocol());
//this->_envVariables[FORMAT] = createEnvString(FORMAT, req.getFormat());
//this->_envVariables[BODY] = createEnvString(BODY, this->_req.getBodyPayload());
//this->_envVariables[HEADER] = createEnvString(HEADER, this->_req.getHeader(???));
//this->_envVariables[LINE] = createEnvString(LINE, this->_req.getLineRequest());
	
	std::map<std::string, std::string>::iterator it;
    for (it = _envVariables.end(); it != _envVariables.begin(); --it) {
        std::cout << it->second << std::endl;
    }
	
}

bool    isCGIRequest(const Request &request) {
    if (request.getUri() == CGI_SCRIPT_PATH) {
        return (true);
    }
    else if (request.getUri() == CGI_UPLOAD_SCRIPT_PATH /*&& request.getProtocol() == "POST"*/) {
        std::cout << "---Form html page---\n";
        return (true);
    }

    std::cout << "It's not a CGI request\r\n";

    return (false);
}

int     handleCGIRequest(const Request &req, int fd) {
    pid_t   pid = 0;
	int 	status = 0;
	int		in_pipe[2];
	int		out_pipe[2];

	Cgi cgi(req, fd);
	char *env[] = {NULL};

	if (pipe(in_pipe) == -1)
		return (-1);
	if (pipe(out_pipe) == -1) {
		close(in_pipe[0]);
		close(in_pipe[1]);
		return (-1);
	}

    pid = fork();
    if (pid == -1)
        return (-1);
    // Child process
	if (!pid) {
        // Close unnecessary ends of the pipes
        close(in_pipe[1]);
        close(out_pipe[0]);
		//Redirect stdin and stdout
		if (::dup2(in_pipe[0], STDIN_FILENO) == -1)
            return (-1);
		close(in_pipe[0]);
        if (::dup2(out_pipe[1], STDOUT_FILENO) == -1) {}
			return (-1);
        close(out_pipe[1]);
		//Execute
		execve(cgi.getExec().c_str(), NULL, env);
		perror("execve");
		//clear_c_env(env); ???
		exit(EXIT_FAILURE);
    }
	// Parent process
    else {
        int			retPid = 0;
		int			ret = 0;
		std::string body = NULL;
		char		buf[BUFFER_LENGTH];
		std::string	toRet = NULL;

		close(in_pipe[0]);
		close(out_pipe[1]);
		body = req.getBodyPayload();
		status = write(in_pipe[1], body.c_str(), body.size());
		if (status < 0)
			return(-1);
		close(in_pipe[1]);
		
		while((ret = read(out_pipe[0], buf, BUFFER_LENGTH))) {
			if (ret < 0)
				return -1;
			toRet = buf;
			status = send(cgi.getSocketFd(), toRet.c_str(), toRet.size(), 0);
		}
		close(out_pipe[0]);
		waitpid(pid, &retPid, 0);
		if (WIFEXITED(retPid) ) {
			HttpResponse rep;
			rep.setErrorResponse(500, "");
			std::string toSend = rep.getResponse();
			status = send(cgi.getSocketFd(), toSend.c_str(), toSend.size(), 0);
		}	
    }
    return (status);
}
