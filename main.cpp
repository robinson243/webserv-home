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

HttpResponse Get(const HttpRequest &req, const ServerConfig &server);

// ─── Helpers ──────────────────────────────────────────────────────────────────

static void mk(const std::string &path) { mkdir(path.c_str(), 0755); }

static void mkfile(const std::string &path, const std::string &content)
{
    std::ofstream f(path.c_str());
    f << content;
    f.close();
}

static void mkfile_empty(const std::string &path)
{
    std::ofstream f(path.c_str());
    f.close();
}

static void setup()
{
    mk("./www");
    mk("./www/files");
    mk("./www/empty_dir");
    mk("./www/dir_index");
    mk("./www/dir_autoindex");
    mk("./www/dir_both");         // index ET autoindex activé
    mk("./www/nested");
    mk("./www/nested/sub");
    mk("./www/no_read");          // dossier sans index ni autoindex

    // Fichiers normaux
    mkfile("./www/files/hello.html",  "<html><body>Hello</body></html>");
    mkfile("./www/files/style.css",   "body { color: red; }");
    mkfile("./www/files/data.json",   "{\"key\":\"value\"}");
    mkfile("./www/files/image.jpg",   "FAKEJPEG");
    mkfile_empty("./www/files/empty.html");

    // Index
    mkfile("./www/dir_index/index.html", "<html><body>Index</body></html>");
    mkfile("./www/dir_both/index.html",  "<html><body>Both</body></html>");

    // Fichiers dans autoindex
    mkfile("./www/dir_autoindex/file1.txt", "hello");
    mkfile("./www/dir_autoindex/file2.txt", "world");

    // Nested
    mkfile("./www/nested/sub/deep.html", "<html>deep</html>");

    // Fichier sans permission de lecture
    mkfile("./www/no_read/secret.html", "secret");
    chmod("./www/no_read/secret.html", 0000);
}

static HttpRequest makeRequest(const std::string &uri)
{
    HttpRequest req;
    std::string raw = "GET " + uri + " HTTP/1.1\r\nHost: localhost\r\n\r\n";
    req.addHttpRequest(raw);
    return req;
}

static LocationConfig makeLocation(const std::string &root,
                                   const std::string &index,
                                   bool autoindex)
{
    LocationConfig loc;
    loc.setPath("/");
    loc.setRoot(root);
    loc.sethasAutoindex(true);
    loc.setAutoindex(autoindex);
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

static ServerConfig makeServer(const std::string &root,
                               const std::string &index,
                               bool autoindex)
{
    ServerConfig server;
    server.setRoot(root);
    server.setLocations(makeLocation(root, index, autoindex));
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

// ─── Tests ────────────────────────────────────────────────────────────────────

int main()
{
    setup();

    std::cout << "\n=== FICHIERS STATIQUES ===" << std::endl;

    // Basique
    check("T01 fichier .html existant",
        Get(makeRequest("/files/hello.html"),
            makeServer("./www", "", false)), 200);

    check("T02 fichier .css existant",
        Get(makeRequest("/files/style.css"),
            makeServer("./www", "", false)), 200);

    check("T03 fichier .json existant",
        Get(makeRequest("/files/data.json"),
            makeServer("./www", "", false)), 200);

    check("T04 fichier .jpg existant",
        Get(makeRequest("/files/image.jpg"),
            makeServer("./www", "", false)), 200);

    check("T05 fichier vide (0 octets)",
        Get(makeRequest("/files/empty.html"),
            makeServer("./www", "", false)), 200);

    // Inexistants
    check("T06 fichier inexistant",
        Get(makeRequest("/files/ghost.html"),
            makeServer("./www", "", false)), 404);

    check("T07 URI completement inventee",
        Get(makeRequest("/nope/nope/nope.html"),
            makeServer("./www", "", false)), 404);

    check("T08 extension inconnue inexistante",
        Get(makeRequest("/files/file.xyz"),
            makeServer("./www", "", false)), 404);

    // Permissions
    check("T09 fichier sans permission lecture",
        Get(makeRequest("/no_read/secret.html"),
            makeServer("./www", "", false)), 403);


    std::cout << "\n=== DOSSIERS + INDEX ===" << std::endl;

    check("T10 dossier avec index.html",
        Get(makeRequest("/dir_index"),
            makeServer("./www", "index.html", false)), 200);

    check("T11 dossier avec index.html + slash final",
        Get(makeRequest("/dir_index/"),
            makeServer("./www", "index.html", false)), 200);

    check("T12 index configure mais inexistant dans ce dossier",
        Get(makeRequest("/dir_autoindex"),
            makeServer("./www", "index.html", false)), 403);

    check("T13 dossier avec index ET autoindex → index prioritaire",
        Get(makeRequest("/dir_both"),
            makeServer("./www", "index.html", true)), 200);


    std::cout << "\n=== AUTOINDEX ===" << std::endl;

    check("T14 dossier sans index, autoindex on → 200",
        Get(makeRequest("/dir_autoindex"),
            makeServer("./www", "", true)), 200);

    check("T15 dossier vide, autoindex on → 200",
        Get(makeRequest("/empty_dir"),
            makeServer("./www", "", true)), 200);

    check("T16 dossier sans index, autoindex off → 403",
        Get(makeRequest("/dir_autoindex"),
            makeServer("./www", "", false)), 403);


    std::cout << "\n=== CAS LIMITES ===" << std::endl;

    check("T17 URI racine '/' avec index",
        Get(makeRequest("/"),
            makeServer("./www/dir_index", "index.html", false)), 200);

    check("T18 URI racine '/' sans index, autoindex on",
        Get(makeRequest("/"),
            makeServer("./www/dir_autoindex", "", true)), 200);

    check("T19 fichier profondement imbrique",
        Get(makeRequest("/nested/sub/deep.html"),
            makeServer("./www", "", false)), 200);

    check("T20 URI avec double slash //files//hello.html",
        Get(makeRequest("//files//hello.html"),
            makeServer("./www", "", false)), 404);

    check("T21 dossier inexistant, autoindex on → 404",
        Get(makeRequest("/fantome/"),
            makeServer("./www", "", true)), 404);


    // Nettoyage permission pour ne pas laisser de fichier inaccessible
    chmod("./www/no_read/secret.html", 0644);

    std::cout << "\n─────────────────────────────────" << std::endl;
    std::cout << "Résultat : " << passed << "/" << (passed + failed)
              << " tests passés" << std::endl;
    if (failed > 0)
        std::cout << "\033[31m" << failed << " test(s) échoué(s)\033[0m" << std::endl;
    else
        std::cout << "\033[32mTous les tests sont passés !\033[0m" << std::endl;

    return (failed > 0 ? 1 : 0);
}
