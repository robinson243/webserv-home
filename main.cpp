#include "HttpRequest.hpp"

#include <sstream>
int main() {
	HttpRequest test;
	// std::string str = "POST /upload/fichier.txt HTTP/1.1\r\n"
	// 				  "Host: localhost:8080\r\n"
	// 				  "Content-Length: 29\r\n"
	// 				  "\r\n"
	// 				  "Bonjour,\n"
	// 				  "voici mon fichier !";
	// std::stringstream ss(str);
	// std::string token;
	// while (ss >> token) {
	// 	std::cout << "word " << token << std::endl;
	// }
	std::string te, le, re;
	te = "test";
	le = "element";
	re = "element2";
	test.addRequest(te, le);
	test.addRequest(te, re);
	for (size_t i = 0; i < test.getRequest().size(); i++)
	{
		std::cout << "key: " << test.getRequest()["test"] << std::endl; 
	}
	

	return 0;
}