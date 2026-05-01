#include "LocationConfig.hpp"
#include "RequestHandler.hpp"
#include <iostream>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::string raw;
    int expected;
};

static void addMethods(LocationConfig &loc,
                       const std::string &a,
                       const std::string &b = "",
                       const std::string &c = "") {
    if (!a.empty())
        loc.addMethod(a);
    if (!b.empty())
        loc.addMethod(b);
    if (!c.empty())
        loc.addMethod(c);
}

static ServerConfig buildServer() {
    ServerConfig server;
    server.setRoot("./output/webserv_get_suite/www");
    server.setIndex(std::vector<std::string>(1, "index.html"));

    LocationConfig root;
    root.setPath("/");
    root.setRoot("./output/webserv_get_suite/www");
    addMethods(root, "DELETE");

    LocationConfig deletezone;
    deletezone.setPath("/deletezone");
    deletezone.setRoot("./output/webserv_get_suite/www/deletezone");
    addMethods(deletezone, "DELETE");

    LocationConfig upload;
    upload.setPath("/upload");
    upload.setRoot("./output/webserv_get_suite/www/upload");
    addMethods(upload, "GET", "POST");

    LocationConfig dir;
    dir.setPath("/dir");
    dir.setRoot("./output/webserv_get_suite/www/dir");
    addMethods(dir, "DELETE");

    server.setLocations(root);
    server.setLocations(deletezone);
    server.setLocations(upload);
    server.setLocations(dir);
    return server;
}

static void run(const TestCase &tc, const ServerConfig &server) {
    HttpRequest req;
    std::string raw = tc.raw;
    req.addHttpRequest(raw);
    HttpResponse resp = Delete(req, server);
    std::cout << (resp.getCode() == tc.expected ? "[PASS] " : "[FAIL] ")
              << tc.name << " (Expected " << tc.expected << ", Got "
              << resp.getCode() << ")\n";
}

int main() {
    ServerConfig server = buildServer();
    std::vector<TestCase> tests;

    tests.push_back((TestCase){
        "DELETE existing file",
        "DELETE /deletezone/delete_me.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        204
    });

    tests.push_back((TestCase){
        "DELETE second existing file",
        "DELETE /deletezone/delete_me_2.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        204
    });

    tests.push_back((TestCase){
        "DELETE missing file",
        "DELETE /deletezone/missing.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        404
    });

    tests.push_back((TestCase){
        "DELETE directory at deletezone",
        "DELETE /deletezone HTTP/1.1\r\nHost: localhost\r\n\r\n",
        403
    });

    tests.push_back((TestCase){
        "DELETE existing directory with slash",
        "DELETE /dir/ HTTP/1.1\r\nHost: localhost\r\n\r\n",
        403
    });

    tests.push_back((TestCase){
        "DELETE traversal direct",
        "DELETE /deletezone/../upload/file1.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        403
    });

    tests.push_back((TestCase){
        "DELETE traversal nested",
        "DELETE /deletezone/sub/../delete_me.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        403
    });

    tests.push_back((TestCase){
        "DELETE unknown location",
        "DELETE /ghost/file.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
        404
    });

    tests.push_back((TestCase){
        "DELETE empty uri",
        "DELETE  HTTP/1.1\r\nHost: localhost\r\n\r\n",
        400
    });

    tests.push_back((TestCase){
        "DELETE uri only slash",
        "DELETE / HTTP/1.1\r\nHost: localhost\r\n\r\n",
        403
    });

    tests.push_back((TestCase){
        "DELETE malformed header",
        "DELETE /deletezone/delete_me.txt HTTP/1.1\r\nHost localhost\r\n\r\n",
        400
    });

    for (size_t i = 0; i < tests.size(); ++i)
        run(tests[i], server);

    return 0;
}