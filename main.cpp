#include "RequestHandler.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include <iostream>
#include <vector>
#include <string>

struct TestCase
{
    std::string name;
    std::string raw;
    int expected;
};

static void addMethods(LocationConfig &loc,
                       const std::string &a,
                       const std::string &b = "",
                       const std::string &c = "")
{
    if (!a.empty())
        loc.addMethod(a);
    if (!b.empty())
        loc.addMethod(b);
    if (!c.empty())
        loc.addMethod(c);
}

static ServerConfig buildServer()
{
    ServerConfig server;
    server.setRoot("./output/handle_request_suite/www");
    server.setIndex(std::vector<std::string>(1, "index.html"));

    LocationConfig root;
    root.setPath("/");
    root.setRoot("./output/handle_request_suite/www");
    addMethods(root, "GET", "DELETE");

    LocationConfig files;
    files.setPath("/files");
    files.setRoot("./output/handle_request_suite/www/files");
    addMethods(files, "GET", "DELETE");

    LocationConfig upload;
    upload.setPath("/upload");
    upload.setRoot("./output/handle_request_suite/www/upload");
    addMethods(upload, "GET", "POST");

    LocationConfig redir;
    redir.setPath("/redir");
    redir.setRoot("./output/handle_request_suite/www");
    redir.setCode(301);
    redir.setUrl("/files/hello.txt");
    addMethods(redir, "GET");

    server.setLocations(root);
    server.setLocations(files);
    server.setLocations(upload);
    server.setLocations(redir);

    return server;
}

static void runTest(const TestCase &tc, const ServerConfig &server)
{
    HttpRequest req;
    std::string raw = tc.raw;
    req.addHttpRequest(raw);

    HttpResponse resp = handleRequest(req, server);

    std::cout << (resp.getCode() == tc.expected ? "[PASS] " : "[FAIL] ")
              << tc.name
              << " (Expected " << tc.expected
              << ", Got " << resp.getCode() << ")"
              << std::endl;
}

int main()
{
    ServerConfig server = buildServer();
    std::vector<TestCase> tests;

    tests.push_back((TestCase){
        "GET existing root file",
        "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n",
        200});

    tests.push_back((TestCase){
        "GET existing file in location",
        "GET /files/hello.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        200});

    tests.push_back((TestCase){
        "GET missing file",
        "GET /files/missing.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        404});

    tests.push_back((TestCase){
        "GET redirect",
        "GET /redir HTTP/1.1\r\nHost: localhost\r\n\r\n",
        301});

    tests.push_back((TestCase){
        "DELETE existing file",
        "DELETE /files/delete_me.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        204});

    tests.push_back((TestCase){
        "DELETE missing file",
        "DELETE /files/nope.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        404});

    tests.push_back((TestCase){
        "DELETE directory forbidden",
        "DELETE /files/subdir HTTP/1.1\r\nHost: localhost\r\n\r\n",
        403});

    tests.push_back((TestCase){
        "POST allowed in upload",
        "POST /upload/new.txt HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello",
        201});

    tests.push_back((TestCase){
        "POST not allowed in files",
        "POST /files/hello.txt HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello",
        405});

    tests.push_back((TestCase){
        "Unknown location",
        "GET /ghost/file.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        404});

    tests.push_back((TestCase){
        "Malformed header",
        "GET /index.html HTTP/1.1\r\nHost localhost\r\n\r\n",
        400});

    tests.push_back((TestCase){
        "Unknown method",
        "PUT /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n",
        501});

    for (size_t i = 0; i < tests.size(); ++i)
        runTest(tests[i], server);

    return 0;
}