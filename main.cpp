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
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "RequestHandler.hpp"
#include <iostream>

int main()
{
    // --- Construire la config ---
    ServerConfig server;

    LocationConfig locRoot;
    locRoot.setPath("/");
    locRoot.setRoot("./www");
    locRoot.addMethod("GET");
    server.setLocations(locRoot);

    LocationConfig locUpload;
    locUpload.setPath("/upload");
    locUpload.setRoot("./www/uploads");
    locUpload.setUploadPath("./www/uploads");
    locUpload.addMethod("POST");
    server.setLocations(locUpload);

    LocationConfig locFiles;
    locFiles.setPath("/files");
    locFiles.setRoot("./www/files");
    locFiles.addMethod("DELETE");
    server.setLocations(locFiles);

    // --- Test 1 : GET valide ---
    std::cout << "=== TEST 1 : GET ===" << std::endl;
    {
        HttpRequest req;
        std::string method = "GET";
        std::string uri = "/index.html";
        req.addRequest("method", method);
        req.addRequest("uri", uri);
        HttpResponse res = handleRequest(req, server);
        std::cout << res.serialize() << std::endl;
    }

    // --- Test 2 : POST valide ---
    std::cout << "=== TEST 2 : POST ===" << std::endl;
    {
        HttpRequest req;
        std::string method = "POST";
        std::string uri = "/upload/test.txt";
        std::string body = "Hello Upload!";
        req.addRequest("method", method);
        req.addRequest("uri", uri);
        req.addBody(body);
        HttpResponse res = handleRequest(req, server);
        std::cout << res.serialize() << std::endl;
    }

    // --- Test 3 : méthode non autorisée ---
    std::cout << "=== TEST 3 : POST sur / (405) ===" << std::endl;
    {
        HttpRequest req;
        std::string method = "POST";
        std::string uri = "/index.html";
        req.addRequest("method", method);
        req.addRequest("uri", uri);
        HttpResponse res = handleRequest(req, server);
        std::cout << res.serialize() << std::endl;
    }

    // --- Test 4 : location introuvable ---
    std::cout << "=== TEST 4 : URI inconnue (404) ===" << std::endl;
    {
        HttpRequest req;
        std::string method = "GET";
        std::string uri = "/unknown/page";
        req.addRequest("method", method);
        req.addRequest("uri", uri);
        HttpResponse res = handleRequest(req, server);
        std::cout << res.serialize() << std::endl;
    }

    // --- Test 5 : méthode inconnue ---
    std::cout << "=== TEST 5 : méthode PATCH (405) ===" << std::endl;
    {
        HttpRequest req;
        std::string method = "PATCH";
        std::string uri = "/index.html";
        req.addRequest("method", method);
        req.addRequest("uri", uri);
        HttpResponse res = handleRequest(req, server);
        std::cout << res.serialize() << std::endl;
    }

    return 0;
}