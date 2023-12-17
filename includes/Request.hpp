#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <iostream>
# include <algorithm>
# include <map>
# include <vector>
# include <sstream>

class Request {
private:
	std::string 							_requestline;
	std::string								_requestprotocol;
	std::string 							_requesturi;
	std::string								_requestformat;

	std::map<std::string, std::string>		_requestheadertypes;

public:
	Request();
	Request(const std::string &requestformat);
	Request &operator=(const Request &other);

	const std::string	&getFormat() const;
	const std::string	&getLineRequest() const;
	const std::string	&getProtocol() const;
	const std::string	&getUri() const;
	const std::string	getHeader(const std::string &headertype) const;

	void		displayHeaderTypes() const;

	void		setupRequest();

	void		setUri();
	void		setProtocol();
	void		setLine();
	void		setHeader();

	bool		isValidProtocol();
	void 		replaceProtocolTo(const std::string &newprotocol);
};

#endif