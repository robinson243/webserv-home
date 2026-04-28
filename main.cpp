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

#include <iostream>
#include "HttpResponse.hpp"

int main()
{
    HttpResponse response;
    std::string version = "HTTP/1.1";
    std::string message = "OK";
    std::string body = "Hello World!";

    response.addCode(200);
    response.addVersion(version);
    response.addMessage(message);
    response.addHeadersResponse("Content-Type", "text/plain");
    response.addHeadersResponse("Content-Length", "12");
    response.addBodyResponse(body);

    std::cout << response.serialize() << std::endl;
    return 0;
}