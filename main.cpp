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
	test.print();

	return 0;
}