#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <algorithm>
#include <iostream>

# define GREEN "\e[32m"
# define COL "\e[36m"
# define NOCOL "\e[39m"
# define RED "\e[91m"

class HttpResponse {
private:
	int						_statuscode;
	unsigned long			_length;
	std::string				_statusmessage;
	std::string				_response;
	std::string				_body;

public:
	HttpResponse();
	~HttpResponse();
	HttpResponse &operator=(const HttpResponse &other);

	void	setNormalResponse(int statuscode, const std::string &statusmessage, const std::string &bodyContent, const std::string &mimeType, const std::string &lastfile);
	void	setErrorResponse(int statuscode, const std::string &statusmessage);
	void	setDeleteResponse(int statusCode, const std::string &statusText);

	const std::string &getResponse() const;
	const std::string &getStatusMessage() const;
	const int		&getStatusCode() const;
	const std::string &getBody() const;

	const unsigned long		&getLength() const;
};

#endif
