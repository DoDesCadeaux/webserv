#include "../includes/HttpResponse.hpp"
#include <iostream>

std::string HttpResponse::getResponse(int statuscode, const std::string &statusmessage, const std::string &bodycontent, const std::string &mimeType, const std::string &lastfile) {
	std::string statusline = "HTTP/1.1 " + std::to_string(statuscode) + " " + statusmessage + "\r\n";
	std::map<std::string, std::string> headers;

	if (statuscode == 302) {
		std::string lastfileName = lastfile.substr(4);
		std::cout << "LAST FILE NAME :" << lastfileName << std::endl;
		headers["Location"] = "http://localhost:8081/" + lastfileName;
	}
	else {
		headers["Content-Type"] = mimeType; // Utilisez le type MIME fourni
		headers["Content-Length"] = std::to_string(bodycontent.size());
		headers["Connection"] = "close";
	}

	std::string response = statusline;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
		response += it->first + ": " + it->second + "\r\n";
	}
	response += "\r\n"; // Ajouter une ligne vide entre les en-têtes et le corps de la réponse
	response += bodycontent;
	return response;
}

std::string HttpResponse::getErrorResponse(int statuscode, const std::string &statusmessage) {
	std::string body = "<html><body><h1>" + statusmessage + "</h1></body></html>";
	std::string response = "HTTP/1.1 " + std::to_string(statuscode) + " " + statusmessage + "\r\n" +
						   "Content-Type: text/html\r\n" +
						   "Content-Length: " + std::to_string(body.length()) + "\r\n" +
						   "\r\n" +
						   body;
	return response;
}
