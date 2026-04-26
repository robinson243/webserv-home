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
#include <fstream>
#include <sys/stat.h>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

HttpResponse Get(const HttpRequest &req, const ServerConfig &server);

// ─── Setup fichiers de test ───────────────────────────────────────────────────

static void setupTestFiles()
{
    mkdir("./www",                0755);
    mkdir("./www/files",          0755);
    mkdir("./www/dir_with_index", 0755);
    mkdir("./www/dir_no_index",   0755);

    std::ofstream f1("./www/files/hello.html");
    f1 << "<html><body>Hello</body></html>";
    f1.close();

    std::ofstream f2("./www/dir_with_index/index.html");
    f2 << "<html><body>Index</body></html>";
    f2.close();
}

// ─── Builder HttpRequest ──────────────────────────────────────────────────────

static HttpRequest makeRequest(const std::string &uri)
{
    HttpRequest req;
    std::string raw = "GET " + uri + " HTTP/1.1\r\nHost: localhost\r\n\r\n";
    req.addHttpRequest(raw);
    return req;
}

// ─── Builder LocationConfig ───────────────────────────────────────────────────

static LocationConfig makeLocation(const std::string &path,
                                   const std::string &root,
                                   const std::string &index,
                                   bool               autoindex)
{
    LocationConfig loc;
    loc.setPath(path);
    loc.setRoot(root);
    loc.setAutoindex(autoindex);
    loc.sethasAutoindex(true);

    if (!index.empty())
    {
        std::vector<std::string> idx;
        idx.push_back(index);
        loc.setIndex(idx);
    }

    std::set<std::string> methods;
    methods.insert("GET");
    loc.setAllowMethods(methods);

    return loc;
}

// ─── Builder ServerConfig ─────────────────────────────────────────────────────

static ServerConfig makeServer(const std::string &root,
                               const std::string &index,
                               bool               autoindex)
{
    ServerConfig server;
    server.setRoot(root);

    LocationConfig loc = makeLocation("/", root, index, autoindex);
    server.setLocations(loc);

    return server;
}

// ─── Affichage ────────────────────────────────────────────────────────────────

static void check(const std::string &label, const HttpResponse &resp, int expected)
{
    int         got    = resp.getCode();
    std::string status = (got == expected) ? "✓ OK  " : "✗ FAIL";
    std::cout << status
              << " [" << label << "]"
              << "  attendu=" << expected
              << "  obtenu="  << got
              << std::endl;
}

// ─── Tests ────────────────────────────────────────────────────────────────────

int main()
{
    setupTestFiles();

    // T1 — Fichier statique existant → 200
    {
        ServerConfig server = makeServer("./www", "", false);
        HttpRequest  req    = makeRequest("/files/hello.html");
        check("T1 fichier statique          ", Get(req, server), 200);
    }

    // T2 — Fichier inexistant → 404
    {
        ServerConfig server = makeServer("./www", "", false);
        HttpRequest  req    = makeRequest("/files/ghost.html");
        check("T2 fichier inexistant         ", Get(req, server), 404);
    }

    // T3 — Dossier avec index.html configuré → 200
    {
        ServerConfig server = makeServer("./www", "index.html", false);
        HttpRequest  req    = makeRequest("/dir_with_index");
        check("T3 dossier + index            ", Get(req, server), 200);
    }

    // T4 — Dossier sans index, autoindex off → 403
    {
        ServerConfig server = makeServer("./www", "", false);
        HttpRequest  req    = makeRequest("/dir_no_index");
        check("T4 dossier sans index/autoindex", Get(req, server), 403);
    }

    // T5 — Dossier sans index, autoindex on → TODO (pas encore implémenté)
    {
        ServerConfig server = makeServer("./www", "", true);
        HttpRequest  req    = makeRequest("/dir_no_index");
        HttpResponse resp   = Get(req, server);
        std::cout << "?  [T5 autoindex on             ]"
                  << "  obtenu=" << resp.getCode()
                  << "  (TODO : listing HTML)" << std::endl;
    }

    // T6 — URI pointe sur "/" avec index configuré → 200
    {
        ServerConfig server = makeServer("./www/dir_with_index", "index.html", false);
        HttpRequest  req    = makeRequest("/");
        check("T6 racine avec index          ", Get(req, server), 200);
    }

    return 0;
}