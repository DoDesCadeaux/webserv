#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <algorithm>

class HttpResponse {
private:
	// int			_statuscode;
	// std::string	_statusmessage;
	// std::string	_response;

public:
	// HttpResponse(int statuscode, const std::string &statusmessage);
	// HttpResponse(int statuscode, const std::string &statusmessage, const std::string &bodyContent, const std::string &mimeType);
	static std::string getResponse(int statuscode, const std::string &statusmessage, const std::string &bodyContent, const std::string &mimeType);
	static std::string getErrorResponse(int statuscode, const std::string &statusmessage);
	// std::string getErrorResponse();
};

#endif
