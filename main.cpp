#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "RequestHandler.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static std::vector<std::string> makeIndex(const std::string &name)
{
    std::vector<std::string> v;
    v.push_back(name);
    return v;
}

static void createFile(const std::string &path, const std::string &content)
{
    std::ofstream f(path.c_str());
    if (f.is_open())
        f << content;
}

static void RUN(int n, const std::string &desc, int expected,
                const std::string &method, const std::string &uri,
                const ServerConfig &server, const std::string &body = "")
{
    HttpRequest req;
    std::string m = method;
    std::string u = uri;
    req.addRequest("method", m);
    req.addRequest("uri", u);
    if (!body.empty())
    {
        std::string b = body;
        req.addBody(b);
    }
    HttpResponse res = handleRequest(req, server);
    std::cout << "[T" << n << "] " << desc << std::endl;
    std::cout << "  Attendu : " << expected << std::endl;
    std::cout << "  Recu    : " << res.serialize() << std::endl;
    std::cout << std::endl;
}

static void setupTestFiles()
{
    createFile("./www/del/to_delete1.txt", "delete me 1");
    createFile("./www/del/to_delete2.txt", "delete me 2");
    createFile("./www/del/to_delete3.txt", "delete me 3");
    createFile("./www/del/keep.txt", "do not delete");
    createFile("./www/nested/page.html", "<html>nested</html>");
    createFile("./www/files/readme.txt", "hello files");
    createFile("./www/files/data.json", "{\"ok\":true}");
    createFile("./www/dir_autoindex/sub.txt", "autoindex content");
    createFile("./www/no_read/secret.txt", "forbidden content");
    chmod("./www/no_read/secret.txt", 0000);
    createFile("./www/dir_with_index/index.html", "<html>with index</html>");
    createFile("./www/dir_both/index.html", "<html>both</html>");
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main()
{
    setupTestFiles();

    ServerConfig server;
    server.setRoot("./www");

    LocationConfig locRoot;
    locRoot.setPath("/");
    locRoot.addMethod("GET");
    locRoot.setIndex(makeIndex("index.html"));
    server.setLocations(locRoot);

    LocationConfig locAbout;
    locAbout.setPath("/about");
    locAbout.addMethod("GET");
    server.setLocations(locAbout);

    LocationConfig locNested;
    locNested.setPath("/nested");
    locNested.addMethod("GET");
    locNested.setIndex(makeIndex("index.html"));
    server.setLocations(locNested);

    LocationConfig locFiles;
    locFiles.setPath("/files");
    locFiles.addMethod("GET");
    locFiles.addMethod("DELETE");
    server.setLocations(locFiles);

    LocationConfig locDel;
    locDel.setPath("/del");
    locDel.addMethod("DELETE");
    server.setLocations(locDel);

    LocationConfig locUpload;
    locUpload.setPath("/upload");
    locUpload.addMethod("POST");
    locUpload.setUploadPath("./www/uploads");
    locUpload.setMaxBody(50);
    locUpload.sethasmaxsize(true); // ← ajouter cette ligne
    server.setLocations(locUpload);

    LocationConfig locAutoindex;
    locAutoindex.setPath("/dir_autoindex");
    locAutoindex.addMethod("GET");
    locAutoindex.setAutoindex(true);
    server.setLocations(locAutoindex);

    LocationConfig locNoIndex;
    locNoIndex.setPath("/dir_no_index");
    locNoIndex.addMethod("GET");
    locNoIndex.setAutoindex(false);
    server.setLocations(locNoIndex);

    LocationConfig locWithIndex;
    locWithIndex.setPath("/dir_with_index");
    locWithIndex.addMethod("GET");
    locWithIndex.setIndex(makeIndex("index.html"));
    locWithIndex.setAutoindex(false);
    server.setLocations(locWithIndex);

    LocationConfig locBoth;
    locBoth.setPath("/dir_both");
    locBoth.addMethod("GET");
    locBoth.setIndex(makeIndex("index.html"));
    locBoth.setAutoindex(true);
    server.setLocations(locBoth);

    LocationConfig locEmpty;
    locEmpty.setPath("/empty_dir");
    locEmpty.addMethod("GET");
    locEmpty.setAutoindex(true);
    server.setLocations(locEmpty);

    LocationConfig locNoRead;
    locNoRead.setPath("/no_read");
    locNoRead.addMethod("GET");
    server.setLocations(locNoRead);

    // ════════════════════════════════════════════════════════════════
    // CAT 1 : GET fichiers statiques (1-9)
    // ════════════════════════════════════════════════════════════════
    std::cout << "══════ GET STATIQUE ══════\n\n";

    RUN(1, "GET /index.html → 200",
        200, "GET", "/index.html", server);
    RUN(2, "GET /nonexistent.html → 404",
        404, "GET", "/nonexistent.html", server);
    RUN(3, "GET /nested/page.html → 200",
        200, "GET", "/nested/page.html", server);
    RUN(4, "GET /files/readme.txt → 200 text/plain",
        200, "GET", "/files/readme.txt", server);
    RUN(5, "GET /files/data.json → 200 application/json",
        200, "GET", "/files/data.json", server);
    RUN(6, "GET /no_read/secret.txt → 403 (chmod 0000)",
        403, "GET", "/no_read/secret.txt", server);
    RUN(7, "GET /dir_autoindex/sub.txt → 200",
        200, "GET", "/dir_autoindex/sub.txt", server);
    RUN(8, "GET /del/keep.txt → 405 (GET interdit sur /del)",
        405, "GET", "/del/keep.txt", server);
    RUN(9, "GET /upload/anything → 405 (GET interdit sur /upload)",
        405, "GET", "/upload/anything", server);

    // ════════════════════════════════════════════════════════════════
    // CAT 2 : GET répertoires (10-19)
    // ════════════════════════════════════════════════════════════════
    std::cout << "══════ GET RÉPERTOIRES ══════\n\n";

    RUN(10, "GET / → 200 (index.html trouvé)",
        200, "GET", "/", server);
    RUN(11, "GET /dir_with_index/ → 200 (index.html trouvé)",
        200, "GET", "/dir_with_index/", server);
    RUN(12, "GET /dir_no_index/ → 403 (pas d'index, autoindex OFF)",
        403, "GET", "/dir_no_index/", server);
    RUN(13, "GET /dir_autoindex/ → 200 (listing HTML)",
        200, "GET", "/dir_autoindex/", server);
    RUN(14, "GET /empty_dir/ → 200 (listing vide)",
        200, "GET", "/empty_dir/", server);
    RUN(15, "GET /dir_both/ → 200 (index prioritaire sur autoindex)",
        200, "GET", "/dir_both/", server);
    RUN(16, "GET /dir_with_index → 301 (sans slash final)",
        301, "GET", "/dir_with_index", server);
    RUN(17, "GET /dir_autoindex → 301 (sans slash final)",
        301, "GET", "/dir_autoindex", server);
    RUN(18, "GET /empty_dir → 301 (sans slash final)",
        301, "GET", "/empty_dir", server);
    RUN(19, "GET /dir_no_index → 301 (sans slash final)",
        301, "GET", "/dir_no_index", server);

    // ════════════════════════════════════════════════════════════════
    // CAT 3 : Sécurité traversal (20-26)
    // ════════════════════════════════════════════════════════════════
    std::cout << "══════ SÉCURITÉ ══════\n\n";

    RUN(20, "GET /../etc/passwd → 403",
        403, "GET", "/../etc/passwd", server);
    RUN(21, "GET /files/../../../etc/passwd → 403",
        403, "GET", "/files/../../../etc/passwd", server);
    RUN(22, "GET /nested/../del/keep.txt → 403",
        403, "GET", "/nested/../del/keep.txt", server);
    RUN(23, "DELETE /../etc/passwd → 403",
        403, "DELETE", "/../etc/passwd", server);
    RUN(24, "POST /upload/../../evil.txt → 403 (traversal filename)",
        403, "POST", "/upload/../../evil.txt", server, "body");
    RUN(25, "GET /files/../../www/index.html → 403",
        403, "GET", "/files/../../www/index.html", server);
    RUN(26, "DELETE /del/../del/keep.txt → 403",
        403, "DELETE", "/del/../del/keep.txt", server);

    // ════════════════════════════════════════════════════════════════
    // CAT 4 : POST upload (27-34)
    // ════════════════════════════════════════════════════════════════
    std::cout << "══════ POST ══════\n\n";

    RUN(27, "POST /upload/file1.txt → 201",
        201, "POST", "/upload/file1.txt", server, "Hello Upload!");
    RUN(28, "POST /upload/file2.txt body vide → 201",
        201, "POST", "/upload/file2.txt", server);
    RUN(29, "POST /upload/big.txt body > 50 octets → 413",
        413, "POST", "/upload/big.txt", server,
        "This body is definitely more than fifty characters long!!!");
    RUN(30, "POST /upload/ → 400 (nom de fichier vide)",
        400, "POST", "/upload/", server, "data");
    RUN(31, "POST /index.html → 405 (POST interdit sur /)",
        405, "POST", "/index.html", server, "data");
    RUN(32, "POST /files/test.txt → 405 (POST interdit sur /files)",
        405, "POST", "/files/test.txt", server, "data");
    RUN(33, "POST /del/test.txt → 405 (POST interdit sur /del)",
        405, "POST", "/del/test.txt", server, "data");
    RUN(34, "POST /upload/valid.bin → 201 (upload binaire)",
        201, "POST", "/upload/valid.bin", server, "\x01\x02\x03");

    // ════════════════════════════════════════════════════════════════
    // CAT 5 : DELETE (35-41)
    // ════════════════════════════════════════════════════════════════
    std::cout << "══════ DELETE ══════\n\n";

    RUN(35, "DELETE /del/to_delete1.txt → 204",
        204, "DELETE", "/del/to_delete1.txt", server);
    RUN(36, "DELETE /del/to_delete2.txt → 204",
        204, "DELETE", "/del/to_delete2.txt", server);
    RUN(37, "DELETE /del/to_delete3.txt → 204",
        204, "DELETE", "/del/to_delete3.txt", server);
    RUN(38, "DELETE /del/to_delete1.txt → 404 (déjà supprimé au T35)",
        404, "DELETE", "/del/to_delete1.txt", server);
    RUN(39, "DELETE /del/nonexistent.txt → 404",
        404, "DELETE", "/del/nonexistent.txt", server);
    RUN(40, "DELETE /del/ → 403 (tentative sur dossier)",
        403, "DELETE", "/del/", server);
    RUN(41, "DELETE /files/readme.txt → 204",
        204, "DELETE", "/files/readme.txt", server);

    // ════════════════════════════════════════════════════════════════
    // CAT 6 : Location matching (42-46)
    // ════════════════════════════════════════════════════════════════
    std::cout << "══════ LOCATION MATCHING ══════\n\n";

    RUN(42, "GET /dir_autoindex/sub.txt → 200 (/dir_autoindex > /)",
        200, "GET", "/dir_autoindex/sub.txt", server);
    RUN(43, "GET /uploadmore/test → 404 (/uploadmore != /upload, matche /)",
        404, "GET", "/uploadmore/test", server);
    RUN(44, "DELETE /files/data.json → 204 (DELETE autorisé sur /files)",
        204, "DELETE", "/files/data.json", server);
    RUN(45, "GET /about/ → 403 (dossier sans index ni autoindex)",
        403, "GET", "/about/", server);
    RUN(46, "GET /nested/ → 404 (index.html absent dans ./www/nested/)",
        404, "GET", "/nested/", server);

    // ════════════════════════════════════════════════════════════════
    // CAT 7 : Edge cases (47-50)
    // ════════════════════════════════════════════════════════════════
    std::cout << "══════ EDGE CASES ══════\n\n";

    RUN(47, "PATCH /index.html → 405",
        405, "PATCH", "/index.html", server);
    RUN(48, "HEAD /index.html → 405",
        405, "HEAD", "/index.html", server);
    RUN(49, "PUT /upload/file.txt → 405",
        405, "PUT", "/upload/file.txt", server, "data");
    RUN(50, "DELETE /del/to_delete2.txt → 404 (déjà supprimé au T36)",
        404, "DELETE", "/del/to_delete2.txt", server);

    chmod("./www/no_read/secret.txt", 0644);
    return 0;
}
