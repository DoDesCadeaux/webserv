#include "../includes/HttpResponse.hpp"
HttpResponse::HttpResponse() {}
HttpResponse::~HttpResponse() {}

// HttpResponse::HttpResponse(int statuscode, const std::string &statusmessage){
// 	_statuscode = statuscode;
// 	_statusmessage = statusmessage;
// 	_response = getErrorResponse();
// }

// HttpResponse::HttpResponse(int statuscode, const std::string &statusmessage, const std::string &bodyContent, const std::string &mimeType){
// 	_statuscode = statuscode;
// 	_statusmessage = statusmessage;
// 	_response = getNormalResponse(bodyContent, mimeType);
// }

void HttpResponse::setNormalResponse(int statuscode, const std::string &statusmessage, const std::string &bodycontent, const std::string &mimeType)
{
	_statuscode = statuscode;
	_statusmessage = statusmessage;

	std::string statusline = "HTTP/1.1 " + std::to_string(_statuscode) + " " + _statusmessage + "\r\n";
	std::map<std::string, std::string> headers;

	headers["Content-Type"] = mimeType; // Utilisez le type MIME fourni
	headers["Content-Length"] = std::to_string(bodycontent.size());
	headers["Connection"] = "close";

	_response = statusline;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		_response += it->first + ": " + it->second + "\r\n";
	}
	_response += "\r\n"; // Ajouter une ligne vide entre les en-têtes et le corps de la réponse
	_response += bodycontent;
}

void HttpResponse::setErrorResponse(int statuscode, const std::string &statusmessage)
{
	_statuscode = statuscode;
	_statusmessage = statusmessage;

	std::string body = "<html><body><h1>" + _statusmessage + "</h1></body></html>";
	_response = "HTTP/1.1 " + std::to_string(_statuscode) + " " + _statusmessage + "\r\n" +
				"Content-Type: text/html\r\n" +
				"Content-Length: " + std::to_string(body.length()) + "\r\n" +
				"\r\n" +
				body;
}

const std::string &HttpResponse::getResponse() const
{
	return _response;
}

const std::string &HttpResponse::getStatusMessage() const
{
	return _statusmessage;
}

const int &HttpResponse::getStatusCode() const
{
	return _statuscode;
}