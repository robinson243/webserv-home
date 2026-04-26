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
#include <cstdlib>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

HttpResponse Delete(const HttpRequest &req, const ServerConfig &server);

// ─── Helpers ──────────────────────────────────────────────────────────────────

static void mk(const std::string &path) { mkdir(path.c_str(), 0755); }

static void mkfile(const std::string &path, const std::string &content)
{
    std::ofstream f(path.c_str());
    f << content;
    f.close();
}

static bool fileExists(const std::string &path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

static void setup()
{
    mk("./www");
    mk("./www/del");
    mk("./www/del/subdir");

    mkfile("./www/del/target.html",    "to delete");
    mkfile("./www/del/protected.html", "protected");
    mkfile("./www/del/double.html",    "double delete");

    chmod("./www/del/protected.html", 0444);
}

static HttpRequest makeRequest(const std::string &method, const std::string &uri)
{
    HttpRequest req;
    std::string raw = method + " " + uri + " HTTP/1.1\r\nHost: localhost\r\n\r\n";
    req.addHttpRequest(raw);
    return req;
}

static LocationConfig makeLocation(const std::string &root, bool allowDelete)
{
    LocationConfig loc;
    loc.setPath("/");
    loc.setRoot(root);
    loc.sethasAutoindex(true);
    loc.setAutoindex(false);

    std::set<std::string> methods;
    methods.insert("GET");
    if (allowDelete)
        methods.insert("DELETE");
    loc.setAllowMethods(methods);
    return loc;
}

static ServerConfig makeServer(const std::string &root, bool allowDelete)
{
    ServerConfig server;
    server.setRoot(root);
    server.setLocations(makeLocation(root, allowDelete));
    return server;
}

// ─── Affichage ────────────────────────────────────────────────────────────────

static int passed = 0;
static int failed = 0;

static void check(const std::string &label, const HttpResponse &resp, int expected)
{
    int got = resp.getCode();
    if (got == expected)
    {
        std::cout << "\033[32m✓ OK  \033[0m [" << label << "]"
                  << "  attendu=" << expected << "  obtenu=" << got << std::endl;
        passed++;
    }
    else
    {
        std::cout << "\033[31m✗ FAIL\033[0m [" << label << "]"
                  << "  attendu=" << expected << "  obtenu=" << got << std::endl;
        failed++;
    }
}

static void checkAlsoDeleted(const std::string &label, const HttpResponse &resp,
                              int expected, const std::string &path)
{
    check(label, resp, expected);
    if (expected == 204 && fileExists(path))
    {
        std::cout << "\033[31m✗ FAIL\033[0m [" << label << " → fichier encore présent sur disque]" << std::endl;
        failed++;
        passed--;
    }
    else if (expected == 204)
    {
        std::cout << "\033[32m✓ OK  \033[0m [" << label << " → fichier bien supprimé du disque]" << std::endl;
        passed++;
    }
}

// ─── Tests ────────────────────────────────────────────────────────────────────

int main()
{
    setup();

    std::cout << "\n=== SUPPRESSION NORMALE ===" << std::endl;

    // T01 — Fichier existant, DELETE autorisé → 204 + vérifie sur disque
    checkAlsoDeleted("T01 suppression normale",
        Delete(makeRequest("DELETE", "/del/target.html"),
               makeServer("./www", true)),
        204, "./www/del/target.html");

    // T02 — Supprimer deux fois le même fichier → 404 au 2e appel
    mkfile("./www/del/double.html", "double");
    Delete(makeRequest("DELETE", "/del/double.html"), makeServer("./www", true));
    check("T02 double suppression → 404 au 2e appel",
        Delete(makeRequest("DELETE", "/del/double.html"),
               makeServer("./www", true)), 404);


    std::cout << "\n=== ERREURS CLIENT ===" << std::endl;

    // T03 — Fichier inexistant → 404
    check("T03 fichier inexistant",
        Delete(makeRequest("DELETE", "/del/ghost.html"),
               makeServer("./www", true)), 404);

    // T04 — DELETE non autorisé dans la location → 405
    check("T04 methode DELETE non autorisee",
        Delete(makeRequest("DELETE", "/del/target.html"),
               makeServer("./www", false)), 405);

    // T05 — Cible est un dossier → 403
    check("T05 cible est un dossier",
        Delete(makeRequest("DELETE", "/del/subdir"),
               makeServer("./www", true)), 403);

    // T06 — Fichier sans permission de suppression → 403
    check("T06 fichier protege en lecture seule",
        Delete(makeRequest("DELETE", "/del/protected.html"),
               makeServer("./www", true)), 403);


    std::cout << "\n=== SECURITE ===" << std::endl;

    // T07 — Path traversal → 403
    check("T07 path traversal /../../../etc/passwd",
        Delete(makeRequest("DELETE", "/../../../etc/passwd"),
               makeServer("./www", true)), 403);

    // T08 — URI inventée hors root → 404
    check("T08 URI hors root inexistante",
        Delete(makeRequest("DELETE", "/nope/nope.html"),
               makeServer("./www", true)), 404);


    // Nettoyage permissions
    chmod("./www/del/protected.html", 0644);

    std::cout << "\n─────────────────────────────────" << std::endl;
    std::cout << "Résultat : " << passed << "/" << (passed + failed)
              << " tests passés" << std::endl;
    if (failed > 0)
        std::cout << "\033[31m" << failed << " test(s) échoué(s)\033[0m" << std::endl;
    else
        std::cout << "\033[32mTous les tests sont passés !\033[0m" << std::endl;

    return (failed > 0 ? 1 : 0);
}