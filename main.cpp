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
	// --- Construction de la config ---
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

	// --- Construction des requêtes de test ---
	std::string raw1 =
		"GET /images/photos/cat.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n";
	HttpRequest req1;
	req1.addHttpRequest(raw1);

	std::string raw2 =
		"GET /images/dog.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n";
	HttpRequest req2;
	req2.addHttpRequest(raw2);

	std::string raw3 = "GET /about.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
	HttpRequest req3;
	req3.addHttpRequest(raw3);

	std::string raw4 =
		"GET /images/photoset/pic.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n";
	HttpRequest req4;
	req4.addHttpRequest(raw4);

	// --- Appels à ta fonction ---
	std::cout << "Test 1 (/images/photos/cat.jpg)  → "
			  << findLocation(server, req1) << " (attendu: 2)" << std::endl;
	std::cout << "Test 2 (/images/dog.jpg)          → "
			  << findLocation(server, req2) << " (attendu: 1)" << std::endl;
	std::cout << "Test 3 (/about.html)              → "
			  << findLocation(server, req3) << " (attendu: 0)" << std::endl;
	std::cout << "Test 4 (/images/photoset/pic.jpg) → "
			  << findLocation(server, req4) << " (attendu: 1)" << std::endl;

	return 0;
}