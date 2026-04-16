#include "HttpRequest.hpp"

#include <sstream>
int main() {
	HttpRequest test;
	std::string str = "POST /upload/fichier.txt HTTP/1.1\r\n"
					  "Host: localhost:8080\r\n"
					  "Content-Length: 29\r\n"
					  "\r\n"
					  "Bonjour,\n"
					  "voici mon fichier !";
	std::stringstream ss(str);
	std::string token;
	while (ss >> token) {
		std::cout << "word " << token << std::endl;
	}

	return 0;
}