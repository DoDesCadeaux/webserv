#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <algorithm>

class HttpResponse {
private:
	int						_statuscode;
	unsigned long			_length;
	std::string				_statusmessage;
	std::string				_response;

public:
	HttpResponse();
	~HttpResponse();

	void	setNormalResponse(int statuscode, const std::string &statusmessage, const std::string &bodyContent, const std::string &mimeType, const std::string &lastfile);
	void	setErrorResponse(int statuscode, const std::string &statusmessage);

	const std::string &getResponse() const;
	const std::string &getStatusMessage() const;
	const int		&getStatusCode() const;

	const unsigned long		&getLength() const;
};

#endif
