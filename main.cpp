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
	root.setIndex(std::vector<std::string>(1, "index.html"));
	root.setAutoindex(false);
	addMethods(root, "GET");

	LocationConfig upload;
	upload.setPath("/upload");
	upload.setRoot("./output/webserv_get_suite/www/upload");
	upload.setUploadPath("./output/webserv_get_suite/www/upload");
	upload.setIndex(std::vector<std::string>(1, "index.html"));
	addMethods(upload, "GET", "POST");

	LocationConfig uploadLimited;
	uploadLimited.setPath("/upload-limit");
	uploadLimited.setRoot("./output/webserv_get_suite/www/upload");
	uploadLimited.setUploadPath("./output/webserv_get_suite/www/upload");
	uploadLimited.setMaxBody(5);
	uploadLimited.sethasmaxsize(true);
	addMethods(uploadLimited, "POST");

	LocationConfig noUpload;
	noUpload.setPath("/noupload");
	noUpload.setRoot("./output/webserv_get_suite/www/noupload");
	addMethods(noUpload, "POST");

	LocationConfig redir;
	redir.setPath("/redir");
	redir.setRoot("./output/webserv_get_suite/www");
	redir.setCode(301);
	redir.setUrl("/dir/");
	addMethods(redir, "POST");

	server.setLocations(root);
	server.setLocations(upload);
	server.setLocations(uploadLimited);
	server.setLocations(noUpload);
	server.setLocations(redir);
	return server;
}

static void run(const TestCase &tc, const ServerConfig &server) {
	HttpRequest req;
	std::string raw = tc.raw;
	req.addHttpRequest(raw);
	HttpResponse resp = Post(req, server);
	std::cout << (resp.getCode() == tc.expected ? "[PASS] " : "[FAIL] ")
			  << tc.name << " (Expected " << tc.expected << ", Got "
			  << resp.getCode() << ")\n";
}

int main() {
	ServerConfig server = buildServer();
	std::vector<TestCase> tests;

	tests.push_back((TestCase){ "POST upload txt",
								"POST /upload/file1.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 5\r\n\r\nhello",
								201 });
	tests.push_back(
		(TestCase){ "POST upload html",
					"POST /upload/page.html HTTP/1.1\r\nHost: "
					"localhost\r\nContent-Length: 13\r\n\r\nhello, world!",
					201 });
	tests.push_back((TestCase){ "POST upload json",
								"POST /upload/data.json HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 2\r\n\r\n{}",
								201 });
	tests.push_back(
		(TestCase){ "POST upload binary-like",
					"POST /upload/bin.dat HTTP/1.1\r\nHost: "
					"localhost\r\nContent-Length: 6\r\n\r\nA\x01B\x02C\x03",
					400 });
	tests.push_back((TestCase){ "POST upload with query",
								"POST /upload/query.txt?x=1 HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 3\r\n\r\nabc",
								201 });
	tests.push_back((TestCase){ "POST upload with fragment",
								"POST /upload/frag.txt#x HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 3\r\n\r\nabc",
								201 });

	tests.push_back((TestCase){
		"POST missing host",
		"POST /upload/nohost.txt HTTP/1.1\r\nContent-Length: 4\r\n\r\ntest",
		400 });
	tests.push_back((TestCase){ "POST empty body",
								"POST /upload/empty.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 0\r\n\r\n",
								400 });
	tests.push_back((TestCase){
		"POST no content length",
		"POST /upload/nolen.txt HTTP/1.1\r\nHost: localhost\r\n\r\nhello",
		400 });
	tests.push_back((TestCase){ "POST bad content length alpha",
								"POST /upload/badlen.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: abc\r\n\r\nhello",
								400 });
	tests.push_back((TestCase){ "POST wrong content length short",
								"POST /upload/wrong1.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 10\r\n\r\nabc",
								400 });
	tests.push_back((TestCase){ "POST wrong content length long",
								"POST /upload/wrong2.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 2\r\n\r\nabcd",
								400 });

	tests.push_back((TestCase){ "POST unknown location",
								"POST /ghost/path.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								500 });
	tests.push_back((TestCase){ "POST root no upload path",
								"POST /file_root.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								500 });
	tests.push_back((TestCase){ "POST uri only slash",
								"POST /upload/ HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								400 });
	tests.push_back((TestCase){
		"POST empty uri",
		"POST  HTTP/1.1\r\nHost: localhost\r\nContent-Length: 4\r\n\r\ntest",
		404 });

	tests.push_back((TestCase){ "POST traversal direct",
								"POST /upload/../hack.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								201 });
	tests.push_back(
		(TestCase){ "POST traversal nested",
					"POST /upload/sub/../hack.txt HTTP/1.1\r\nHost: "
					"localhost\r\nContent-Length: 4\r\n\r\ntest",
					201 });
	tests.push_back((TestCase){ "POST double slash filename",
								"POST /upload//double.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 2\r\n\r\nok",
								201 });
	tests.push_back((TestCase){ "POST uppercase file",
								"POST /upload/UPPER.TXT HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								201 });
	tests.push_back((TestCase){ "POST dot file",
								"POST /upload/.hidden HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								201 });

	tests.push_back(
		(TestCase){ "POST chunked header",
					"POST /upload/chunked.txt HTTP/1.1\r\nHost: "
					"localhost\r\nTransfer-Encoding: chunked\r\n\r\nhello",
					201 });
	tests.push_back(
		(TestCase){ "POST chunked no body",
					"POST /upload/chunkempty.txt HTTP/1.1\r\nHost: "
					"localhost\r\nTransfer-Encoding: chunked\r\n\r\n",
					400 });
	tests.push_back((TestCase){ "POST http 1.0",
								"POST /upload/v10.txt HTTP/1.0\r\nHost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								201 });
	tests.push_back((TestCase){ "POST unknown method typo",
								"POTS /upload/x.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								404 });
	tests.push_back((TestCase){ "POST malformed header",
								"POST /upload/badheader.txt HTTP/1.1\r\nHost "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								400 });

	tests.push_back((TestCase){ "POST lowercase host header",
								"POST /upload/case1.txt HTTP/1.1\r\nhost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								400 });
	tests.push_back((TestCase){ "POST lowercase content-length",
								"POST /upload/case2.txt HTTP/1.1\r\nHost: "
								"localhost\r\ncontent-length: 4\r\n\r\ntest",
								400 });
	tests.push_back((TestCase){ "POST upload limited ok",
								"POST /upload-limit/ok.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 5\r\n\r\nhello",
								201 });
	tests.push_back((TestCase){ "POST upload limited too big",
								"POST /upload-limit/big.txt HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 6\r\n\r\nhelloo",
								413 });
	tests.push_back((TestCase){ "POST redir location",
								"POST /redir HTTP/1.1\r\nHost: "
								"localhost\r\nContent-Length: 4\r\n\r\ntest",
								301 });
	for (size_t i = 0; i < tests.size(); ++i)
		run(tests[i], server);

	return 0;
}