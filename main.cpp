#include "HttpRequest.hpp"

int main() {
	HttpRequest test;
	std::string request = "POST /upload HTTP/1.1\r\n"
						  "Host: localhost:8080\r\n"
						  "Content-Type: text/plain\r\n"
						  "Content-Length: 13\r\n"
						  "Connection: close\r\n"
						  "\r\n"
						  "Hello, world!";
	std::stringstream str(request);
	test.addRequestLine(str);
	test.addAllHeaders(str);
	std::string line;
	std::getline(str, line);
	test.addBody(line);
	// std::map<std::string, std::string>::const_iterator it;
	// for (it = test.getHeaders().begin(); it != test.getHeaders().end(); ++it)
	// {
	// 	std::cout << "key: " << it->first << " value: " << it->second <<
	// std::endl;
	// }
	std::vector<unsigned char> body = test.getBody();
	std::cout << std::string(body.begin(), body.end());
	// std::cout << "word :" << token << std::endl;
	// while (std::getline(str, token))
	// {
	// 	std::cout << "word :" << token << std::endl;
	// 	if (!token.empty() && token[token.size() - 1] == '\r')
	// 		token.erase(token.size() - 1);
	// }

	return 0;
}