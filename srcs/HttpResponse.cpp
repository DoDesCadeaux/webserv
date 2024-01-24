#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse() {
	_statuscode = 0;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setDeleteResponse(int statusCode, const std::string &statusText)
{
	_statuscode = statusCode;
	_statusmessage = statusText;
	_response = "HTTP/1.1 " + std::to_string(statusCode) + " " + statusText + "\r\n\r\n";
}

void HttpResponse::setNormalResponse(int statuscode, const std::string &statusmessage, const std::string &bodycontent, const std::string &mimeType, const std::string &lastfile)
{
	_statuscode = statuscode;
	_statusmessage = statusmessage;

	std::string statusline = "HTTP/1.1 " + std::to_string(_statuscode) + " " + _statusmessage + "\r\n";
	std::map<std::string, std::string> headers;

	if (statuscode == 302) {
		headers["Location"] = "/" + lastfile;
	}
	else {
		headers["Content-Type"] = mimeType;
		headers["Content-Length"] = std::to_string(bodycontent.size());
		headers["Connection"] = "close";
	}

	_length = bodycontent.length();

	_response = statusline;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		_response += it->first + ": " + it->second + "\r\n";
	}
	_response += "\r\n";
	_response += bodycontent;
	_body = bodycontent;
}

void HttpResponse::setErrorResponse(int statuscode, const std::string &statusmessage)
{
	_statuscode = statuscode;
	_statusmessage = statusmessage;

	_body = "<html><body><h1> Error: " + std::to_string(_statuscode) + " " +  _statusmessage + "</h1></body></html>";
	_response = "HTTP/1.1 " + std::to_string(_statuscode) + " " + _statusmessage + "\r\n" +
				"Content-Type: text/html\r\n" +
				"Content-Length: " + std::to_string(_body.length()) + "\r\n" +
				"\r\n" +
				_body;
	_length = _body.length();
}

const std::string &HttpResponse::getResponse() const
{
	return _response;
}

const std::string &HttpResponse::getStatusMessage() const
{
	return _statusmessage;
}

const std::string &HttpResponse::getBody() const
{
	return _body;
}

const int &HttpResponse::getStatusCode() const
{
	return _statuscode;
}

const unsigned long &HttpResponse::getLength() const {
	return _length;
}