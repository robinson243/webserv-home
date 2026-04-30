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
    server.setRoot("./output/webserv_delete_suite/www");
    server.setIndex(std::vector<std::string>(1, "index.html"));

    LocationConfig root;
    root.setPath("/");
    root.setRoot("./output/webserv_delete_suite/www");
    root.setIndex(std::vector<std::string>(1, "index.html"));
    addMethods(root, "DELETE");

    LocationConfig files;
    files.setPath("/files");
    files.setRoot("./output/webserv_delete_suite/www/files");
    addMethods(files, "DELETE");

    LocationConfig locked;
    locked.setPath("/locked");
    locked.setRoot("./output/webserv_delete_suite/www/locked");
    addMethods(locked, "DELETE");

    LocationConfig noDelete;
    noDelete.setPath("/nodelete");
    noDelete.setRoot("./output/webserv_delete_suite/www/nodelete");
    addMethods(noDelete, "GET");

    server.setLocations(root);
    server.setLocations(files);
    server.setLocations(locked);
    server.setLocations(noDelete);
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

    tests.push_back((TestCase){ "DELETE existing file",
                                "DELETE /files/delete_me.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                204 });

    tests.push_back((TestCase){ "DELETE missing file",
                                "DELETE /files/missing.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                404 });

    tests.push_back((TestCase){ "DELETE root existing file",
                                "DELETE /delete_root.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                204 });

    tests.push_back((TestCase){ "DELETE root missing file",
                                "DELETE /missing_root.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                404 });

    tests.push_back((TestCase){ "DELETE directory",
                                "DELETE /files/subdir HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                403 });

    tests.push_back((TestCase){ "DELETE locked directory",
                                "DELETE /locked HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                403 });

    tests.push_back((TestCase){ "DELETE path traversal",
                                "DELETE /files/../hack.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                403 });

    tests.push_back((TestCase){ "DELETE path traversal nested",
                                "DELETE /files/sub/../hack.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                403 });

    tests.push_back((TestCase){ "DELETE unknown location",
                                "DELETE /ghost/file.txt HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                404 });

    tests.push_back((TestCase){ "DELETE empty uri",
                                "DELETE  HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                404 });

    tests.push_back((TestCase){ "DELETE uri only slash",
                                "DELETE / HTTP/1.1\r\nHost: localhost\r\n\r\n",
                                403 });

    tests.push_back((TestCase){ "DELETE malformed header",
                                "DELETE /files/delete_me.txt HTTP/1.1\r\nHost localhost\r\n\r\n",
                                400 });

    for (size_t i = 0; i < tests.size(); ++i)
        run(tests[i], server);

    return 0;
}