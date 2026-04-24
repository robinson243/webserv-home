#include "HttpRequest.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

// int main() {
// 	HttpRequest test;
// 	std::string request = "POST /upload HTTP/1.1\r\n"
// 						  "Host: localhost:8080\r\n"
// 						  "Content-Type: text/plain\r\n"
// 						  "Content-Length: 13\r\n"
// 						  "Connection: close\r\n"
// 						  "\r\n"
// 						  "Hello, world!";

// 	test.addHttpRequest(request);
// 	test.print();

// 	return 0;
// }

#include "HttpRequest.hpp"
#include "LocationConfig.hpp"
#include "RequestHandler.hpp"
#include "ServerConfig.hpp"
#include <iostream>

int main() {
	ServerConfig server;
	server.setRoot("/var/www");

	LocationConfig loc1;
	loc1.setPath("/");
	server.setLocations(loc1);

	LocationConfig loc2;
	loc2.setPath("/images");
	server.setLocations(loc2);

	LocationConfig loc3;
	loc3.setPath("/images/photos");
	server.setLocations(loc3);

	HttpRequest req1;
	std::string request1 =
		"GET /images/photos/cat.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n";
	req1.addHttpRequest(request1);

	HttpRequest req2;
	std::string request2 =
		"GET /images/dog.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n";
	req2.addHttpRequest(request2);

	HttpRequest req3;
	std::string request3 =
		"GET /about.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
	req3.addHttpRequest(request3);

	HttpRequest req4;
	std::string request4 =
		"GET /images/photoset/pic.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n";
	req4.addHttpRequest(request4);

	std::cout << concatenatePath(server, req1) << std::endl;
	std::cout << concatenatePath(server, req2) << std::endl;
	std::cout << concatenatePath(server, req3) << std::endl;
	std::cout << concatenatePath(server, req4) << std::endl;

	return 0;
}