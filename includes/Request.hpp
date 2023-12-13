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
	std::string								_requestformat;
	std::string 							_requestline;
	std::string								_requestprotocol;
	std::map<std::string, std::string>		_requestheadertypes;

public:
	Request();
	Request(const std::string &requestformat);

	std::string	getFormat() const;
	std::string	getLine() const;
	std::string getProtocol() const;

	void 		displayHeaderTypes() const;

	void		setProtocol();
	void 		replaceProtocolTo(const std::string &newprotocol);
	void		setLine();
	void		setHeader();

	bool		isValidProtocol();
};

#endif