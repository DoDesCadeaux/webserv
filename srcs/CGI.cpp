#include "CGI.hpp"

//Destructor
Cgi::~Cgi() {
	this->_envVariables.clear();
}

//Default constructor
Cgi::Cgi() {}

//Constructor
Cgi::Cgi(Request req, int socketFd) {
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
** Initialise all the environment variables for the cgi.
*/
void    Cgi::initEnv() {
    this->_envVariables[LINE] = createEnvString(LINE, this->_req.getLineRequest());
    this->_envVariables[PROTOCOL] = createEnvString(PROTOCOL, this->_req.getProtocol());
    this->_envVariables[URI] = createEnvString(URI, this->_req.getUri());
    this->_envVariables[FORMAT] = createEnvString(FORMAT, this->_req.getFormat());
    this->_envVariables[BODY] = createEnvString(BODY, this->_req.getBodyPayload());
//    this->_envVariables[HEADER] = createEnvString(HEADER, this->_req.getHeader(???));
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

bool      handleCGIRequest(int fd) {
    int     status = 0;
    pid_t   pid;

    pid = fork();
    if (pid < 0) {          // Handle error
        return (1);
    }
    else if (pid == 0) {    // Child process
        //Redirect stdin and stdout
        dup2(fd, 0);
        dup2(fd, 1);

        //Set env variables
            //server::setEnv() ??

        //Execute CGI
            //execve() ??
            //protect execve()

    }
    else {                  // Parent process
        waitpid(pid, &status, 0);
    }
    return (status); // ??
}