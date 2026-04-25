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
// }#include <iostream>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "RequestHandler.hpp"

int main()
{
    std::cerr << "MAIN 1: ServerConfig" << std::endl;
    ServerConfig server;
    server.setRoot("./www");

    std::cerr << "MAIN 2: LocationConfig" << std::endl;
    LocationConfig loc;
    std::vector<std::string> indexes;
    indexes.push_back("index.html");
    loc.setIndex(indexes);
    loc.setAutoindex(false);
    loc.setPath("/");
    server.setLocations(loc);

    std::cerr << "MAIN 3: HttpRequest" << std::endl;
    HttpRequest req;

    std::cerr << "MAIN 4: addHttpRequest" << std::endl;
    std::string request = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    req.addHttpRequest(request);

    std::cerr << "MAIN 5: calling Get()" << std::endl;
    HttpResponse response = Get(req, server);

    std::cerr << "MAIN 6: response code = " << response.getCode() << std::endl;

    std::map<std::string, std::string> headers = response.getHeaders();
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
        std::cout << it->first << ": " << it->second << std::endl;

    std::vector<unsigned char> body = response.getBody();
    std::string str(body.begin(), body.end());
    std::cout << "\nBody:\n" << str << std::endl;

    return 0;
}