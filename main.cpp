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

	test.addHttpRequest(request);
	// std::map<std::string, std::string>::const_iterator it;
	// for (it = test.getHeaders().begin(); it != test.getHeaders().end(); ++it)
	// { 	std::cout << "key: " << it->first << " value: " << it->second
	// 			  << std::endl;
	// }
	// std::map<std::string, std::string>::const_iterator it2;
	// for (it2 = test.getRequest().begin(); it2 != test.getRequest().end();
	// 	 ++it2) {
	// 	std::cout << "key: " << it2->first << " value: " << it2->second
	// 			  << std::endl;
	// }

	// std::vector<unsigned char> body = test.getBody();
	// std::cout << std::string(body.begin(), body.end());
	// std::cout << "word :" << token << std::endl;
	// while (std::getline(str, token))
	// {
	// 	std::cout << "word :" << token << std::endl;
	// 	if (!token.empty() && token[token.size() - 1] == '\r')
	// 		token.erase(token.size() - 1);
	// }

	return 0;
}