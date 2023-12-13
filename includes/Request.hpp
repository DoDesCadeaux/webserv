#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <iostream>

class Request {
	private:
		std::string			_requestformat;
	public:
		Request();
		Request(const std::string &_requestformat);
		~Request();
};

#endif