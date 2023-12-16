#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <algorithm>

class HttpResponse {
private:
	HttpResponse();

public:
	static std::string getResponse(int statuscode, const std::string &statusmessage, const std::string &bodyContent, const std::string &mimeType);

};

#endif
