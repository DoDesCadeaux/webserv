#include "../includes/HttpResponse.hpp"

std::string HttpResponse::getResponse(int statuscode, const std::string &statusmessage, const std::string &bodycontent) {
	std::string statusline = "HTTP/1.1 " + std::to_string(statuscode) + " " + statusmessage + "\r\n";
	std::map<std::string, std::string> headers;

	headers["Content-Type"] = "text/html";
	headers["Content-Length"] = std::to_string(bodycontent.size());
	headers["Connection"] = "close";

	std::string response = statusline;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
		response += it->first + ": " + it->second + "\r\n";
	}
	response += bodycontent  + "\r\n";
	return response;
}
