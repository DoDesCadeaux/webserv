#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <iostream>

class Request {
	private:
		std::string			_buff;
	public:
		Request();
		Request(const std::string &buff);
		~Request();
};

#endif