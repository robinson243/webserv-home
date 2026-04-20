#include "HttpRequest.hpp"

#include <sstream>
int main()
{
	HttpRequest test;
	std::string request =
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"Connection: close\r\n"
		"\r\n"
		"Hello, world!";
	std::stringstream str(request);
	std::string token;
	while (str >> token)
	{
		std::cout << "word :" << token << std::endl;
	}
	

	return 0;
}