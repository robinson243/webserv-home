#include "LocationConfig.hpp"
#include "RequestHandler.hpp"
#include "ServerConfig.hpp"
#include <cerrno>

bool isCgiRequest(const HttpRequest &req, const LocationConfig &loc) {
	std::string uri = req.getRequest().at("uri");
	std::string cgiFile;
	std::string ext;
	size_t pos = uri.find("?");
	if (pos == std::string::npos)
		cgiFile = uri;
	else
		cgiFile = uri.substr(0, pos);

	size_t postExt = cgiFile.find_last_of(".");
	if (postExt != std::string::npos) {
		ext = cgiFile.substr(postExt);
		std::map<std::string, std::string>::const_iterator it;
		for (it = loc.getCgiExtension().begin();
			 it != loc.getCgiExtension().end();
			 ++it) {
			if (it->first == ext)
				return true;
		}
	}
	return false;
}

HttpResponse parseCgiOutput(const std::vector<unsigned char> &output) {
	// Convertir en string pour trouver la séparation headers/body
	std::string raw(output.begin(), output.end());

	// Chercher la ligne vide qui sépare headers et body
	std::string sep = "\r\n\r\n";
	size_t sepPos = raw.find(sep);

	// fallback si le CGI utilise \n\n au lieu de \r\n\r\n
	if (sepPos == std::string::npos) {
		sep = "\n\n";
		sepPos = raw.find(sep);
	}

	if (sepPos == std::string::npos)
		return makeResponse(502); // pas de séparateur -> réponse CGI invalide

	std::string headerSection = raw.substr(0, sepPos);
	std::string bodySection = raw.substr(sepPos + sep.size());

	// Parser les headers CGI ligne par ligne
	HttpResponse r;
	int code = 200;
	std::string line;
	std::istringstream stream(headerSection);

	while (std::getline(stream, line)) {
		// Supprimer le \r si présent
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 2);
		if (colon + 1 < line.size())
			value = line.substr(colon + 1);
		if (!value.empty() && value[0] == ' ')
			value = value.substr(1);

		// Header spécial CGI : Status -> code HTTP
		if (key == "Status") {
			// "Status: 200 OK" -> extraire 200
			std::istringstream ss(value);
			ss >> code;
		} else {
			r.addHeadersResponse(key, value);
		}
	}

	// Construire le body
	std::vector<unsigned char> bodyVec(bodySection.begin(), bodySection.end());

	// Content-Length
	std::ostringstream oss;
	oss << bodyVec.size();
	r.addHeadersResponse("Content-Length", oss.str());

	r.addCode(code);
	r.setBody(bodyVec);

	return r;
}

void freeEnvp(char **envp, size_t size) {
	for (size_t i = 0; i < size; ++i)
		delete[] envp[i];
	delete[] envp;
}

HttpResponse handleCgi(const HttpRequest &req,
					   const LocationConfig &loc,
					   const std::string &ext) {
	std::cerr << "\n========== handleCgi ==========" << std::endl;

	std::string REQUEST_METHOD = req.getRequest().at("method");
	std::string interpretor = loc.getCgiExtension().at(ext);
	std::string SCRIPT_NAME;
	std::string scriptPath;
	std::string uri = req.getRequest().at("uri");

	std::cerr << "[CGI] method=" << REQUEST_METHOD << std::endl;
	std::cerr << "[CGI] uri=" << uri << std::endl;
	std::cerr << "[CGI] ext=" << ext << std::endl;
	std::cerr << "[CGI] interpreter(from config)=" << interpretor << std::endl;

	size_t pos = uri.find("?");
	if (pos == std::string::npos)
		SCRIPT_NAME = uri;
	else
		SCRIPT_NAME = uri.substr(0, pos);

	std::string QUERY_STRING;
	if (pos != std::string::npos)
		QUERY_STRING = uri.substr(pos + 1);
	else
		QUERY_STRING = "";

	std::ostringstream s;
	s << req.getBody().size();
	std::string CONTENT_LENGTH = s.str();

	std::cerr << "[CGI] script_name=" << SCRIPT_NAME << std::endl;
	std::cerr << "[CGI] query_string=" << QUERY_STRING << std::endl;
	std::cerr << "[CGI] req body size=" << req.getBody().size() << std::endl;
	std::cerr << "[CGI] content_length=" << CONTENT_LENGTH << std::endl;

	if (!loc.getPath().empty()) {
		bool isPrefix =
			SCRIPT_NAME.size() >= loc.getPath().size()
			&& SCRIPT_NAME.compare(0, loc.getPath().size(), loc.getPath()) == 0;

		bool hasValidBoundary = SCRIPT_NAME.size() == loc.getPath().size()
								|| SCRIPT_NAME[loc.getPath().size()] == '/';

		std::cerr << "[CGI] loc.path=" << loc.getPath() << std::endl;
		std::cerr << "[CGI] loc.root=" << loc.getRoot() << std::endl;
		std::cerr << "[CGI] loc.alias=" << loc.getAlias() << std::endl;
		std::cerr << "[CGI] isPrefix=" << isPrefix
				  << " hasValidBoundary=" << hasValidBoundary << std::endl;

		if (isPrefix && hasValidBoundary) {
			std::string relativePath = SCRIPT_NAME.substr(loc.getPath().size());

			if (relativePath.empty())
				relativePath = "/";

			if (!loc.getAlias().empty()) {
				scriptPath = loc.getAlias() + relativePath;
				std::cerr << "[CGI] relativePath=" << relativePath << std::endl;
				std::cerr << "[CGI] scriptPath via alias=" << scriptPath
						  << std::endl;
			} else {
				scriptPath = loc.getRoot() + relativePath;
				std::cerr << "[CGI] relativePath=" << relativePath << std::endl;
				std::cerr << "[CGI] scriptPath via root=" << scriptPath
						  << std::endl;
			}
		}
	}

	if (scriptPath.empty()) {
		std::cerr << "[CGI][ERROR] scriptPath empty -> 500" << std::endl;
		return makeResponse(500);
	}

	std::string CONTENT_TYPE = "text/plain";
	std::map<std::string, std::string>::const_iterator itCt =
		req.getRequest().find("content-type");
	if (itCt != req.getRequest().end() && !itCt->second.empty())
		CONTENT_TYPE = itCt->second;

	std::string SERVER_PROTOCOL = "HTTP/1.1";
	std::map<std::string, std::string>::const_iterator itVer =
		req.getRequest().find("version");
	if (itVer != req.getRequest().end() && !itVer->second.empty())
		SERVER_PROTOCOL = itVer->second;

	std::vector<std::string> envpVec;
	envpVec.push_back("REQUEST_METHOD=" + REQUEST_METHOD);
	envpVec.push_back("QUERY_STRING=" + QUERY_STRING);
	envpVec.push_back("CONTENT_LENGTH=" + CONTENT_LENGTH);
	envpVec.push_back("CONTENT_TYPE=" + CONTENT_TYPE);
	envpVec.push_back("SCRIPT_NAME=" + SCRIPT_NAME);
	envpVec.push_back("PATH_INFO=" + SCRIPT_NAME);
	envpVec.push_back("PATH_TRANSLATED=" + scriptPath);
	envpVec.push_back("SERVER_PROTOCOL=" + SERVER_PROTOCOL);
	envpVec.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envpVec.push_back("SERVER_SOFTWARE=webserv/1.0");
	envpVec.push_back("SERVER_NAME=localhost");
	envpVec.push_back("SERVER_PORT=8080");
	envpVec.push_back("REQUEST_URI=" + uri);
	envpVec.push_back("SCRIPT_FILENAME=" + scriptPath);

	std::map<std::string, std::string>::const_iterator itHost =
		req.getRequest().find("host");
	if (itHost != req.getRequest().end())
		envpVec.push_back("HTTP_HOST=" + itHost->second);

	std::map<std::string, std::string>::const_iterator itUa =
		req.getRequest().find("user-agent");
	if (itUa != req.getRequest().end())
		envpVec.push_back("HTTP_USER_AGENT=" + itUa->second);

	std::map<std::string, std::string>::const_iterator itAccept =
		req.getRequest().find("accept");
	if (itAccept != req.getRequest().end())
		envpVec.push_back("HTTP_ACCEPT=" + itAccept->second);

	std::map<std::string, std::string>::const_iterator itConn =
		req.getRequest().find("connection");
	if (itConn != req.getRequest().end())
		envpVec.push_back("HTTP_CONNECTION=" + itConn->second);

	char **envp = new char *[envpVec.size() + 1];
	for (size_t i = 0; i < envpVec.size(); ++i) {
		envp[i] = new char[envpVec[i].size() + 1];
		strcpy(envp[i], envpVec[i].c_str());
		std::cerr << "[CGI][ENV] " << envpVec[i] << std::endl;
	}
	envp[envpVec.size()] = NULL;

	char *argv[] = { (char *)interpretor.c_str(),
					 (char *)scriptPath.c_str(),
					 NULL };

	std::cerr << "[CGI][ARGV] argv0=" << argv[0] << std::endl;
	std::cerr << "[CGI][ARGV] argv1=" << argv[1] << std::endl;

	int pipefdIn[2];
	int pipefdOut[2];
	pid_t pid;

	if (pipe(pipefdIn) == -1) {
		std::cerr << "[CGI][ERROR] pipe stdin failed errno=" << errno
				  << " msg=" << strerror(errno) << std::endl;
		return freeEnvp(envp, envpVec.size()), makeResponse(500);
	}

	if (pipe(pipefdOut) == -1) {
		std::cerr << "[CGI][ERROR] pipe stdout failed errno=" << errno
				  << " msg=" << strerror(errno) << std::endl;
		close(pipefdIn[0]);
		close(pipefdIn[1]);
		return freeEnvp(envp, envpVec.size()), makeResponse(500);
	}

	int saved_stdin = dup(STDIN_FILENO);
	int saved_stdout = dup(STDOUT_FILENO);

	std::cerr << "[CGI] pipes created in=(" << pipefdIn[0] << "," << pipefdIn[1]
			  << ") out=(" << pipefdOut[0] << "," << pipefdOut[1] << ")"
			  << std::endl;

	pid = fork();
	if (pid == -1) {
		std::cerr << "[CGI][ERROR] fork failed errno=" << errno
				  << " msg=" << strerror(errno) << std::endl;
		close(pipefdIn[0]);
		close(pipefdIn[1]);
		close(pipefdOut[0]);
		close(pipefdOut[1]);
		close(saved_stdin);
		close(saved_stdout);
		return freeEnvp(envp, envpVec.size()), makeResponse(500);
	}

	if (pid == 0) {
		if (dup2(pipefdIn[0], STDIN_FILENO) == -1)
			_exit(1);
		if (dup2(pipefdOut[1], STDOUT_FILENO) == -1)
			_exit(1);

		close(saved_stdin);
		close(saved_stdout);
		close(pipefdIn[0]);
		close(pipefdIn[1]);
		close(pipefdOut[0]);
		close(pipefdOut[1]);

		std::cerr << "[CGI][EXEC] path=" << interpretor << std::endl;
		std::cerr << "[CGI][EXEC] argv0=" << argv[0] << std::endl;
		std::cerr << "[CGI][EXEC] argv1=" << argv[1] << std::endl;

		execve(interpretor.c_str(), argv, envp);

		std::cerr << "[CGI][EXEC][ERROR] errno=" << errno
				  << " msg=" << strerror(errno) << std::endl;
		_exit(1);
	}

	std::cerr << "[CGI] child pid=" << pid << std::endl;

	close(pipefdIn[0]);
	close(pipefdOut[1]);

	const std::string &method = req.getRequest().at("method");
	if (method == "POST" && !req.getBody().empty()) {
		const std::vector<unsigned char> &body = req.getBody();
		std::cerr << "[CGI][WRITE] trying to write body bytes=" << body.size()
				  << std::endl;

		size_t totalWritten = 0;
		while (totalWritten < body.size()) {
			ssize_t written = write(
				pipefdIn[1],
				reinterpret_cast<const char *>(body.data() + totalWritten),
				body.size() - totalWritten);

			if (written == -1) {
				std::cerr << "[CGI][ERROR] write failed errno=" << errno
						  << " msg=" << strerror(errno) << std::endl;
				close(pipefdIn[1]);
				close(pipefdOut[0]);
				kill(pid, SIGKILL);
				waitpid(pid, NULL, 0);
				dup2(saved_stdin, STDIN_FILENO);
				dup2(saved_stdout, STDOUT_FILENO);
				close(saved_stdin);
				close(saved_stdout);
				return freeEnvp(envp, envpVec.size()), makeResponse(500);
			}

			if (written == 0) {
				std::cerr << "[CGI][ERROR] write returned 0 unexpectedly"
						  << std::endl;
				close(pipefdIn[1]);
				close(pipefdOut[0]);
				kill(pid, SIGKILL);
				waitpid(pid, NULL, 0);
				dup2(saved_stdin, STDIN_FILENO);
				dup2(saved_stdout, STDOUT_FILENO);
				close(saved_stdin);
				close(saved_stdout);
				return freeEnvp(envp, envpVec.size()), makeResponse(500);
			}

			totalWritten += static_cast<size_t>(written);
			std::cerr << "[CGI][WRITE] chunk=" << written
					  << " total=" << totalWritten << "/" << body.size()
					  << std::endl;
		}
	}
	close(pipefdIn[1]);

	std::vector<unsigned char> cgiOutput;
	char buf[4096];
	ssize_t n;
	time_t start = time(NULL);
	const int CGI_TIMEOUT = 5;

	while (true) {
		if (time(NULL) - start > CGI_TIMEOUT) {
			std::cerr << "[CGI][ERROR] timeout after " << CGI_TIMEOUT
					  << "s -> 504" << std::endl;
			close(pipefdOut[0]);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			dup2(saved_stdin, STDIN_FILENO);
			dup2(saved_stdout, STDOUT_FILENO);
			close(saved_stdin);
			close(saved_stdout);
			return freeEnvp(envp, envpVec.size()), makeResponse(504);
		}

		n = read(pipefdOut[0], buf, sizeof(buf));
		if (n == -1) {
			std::cerr << "[CGI][ERROR] read failed errno=" << errno
					  << " msg=" << strerror(errno) << std::endl;
			close(pipefdOut[0]);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			dup2(saved_stdin, STDIN_FILENO);
			dup2(saved_stdout, STDOUT_FILENO);
			close(saved_stdin);
			close(saved_stdout);
			return freeEnvp(envp, envpVec.size()), makeResponse(500);
		}
		if (n == 0)
			break;

		std::cerr << "[CGI][READ] chunk=" << n << std::endl;
		cgiOutput.insert(cgiOutput.end(), buf, buf + n);
	}

	close(pipefdOut[0]);

	std::cerr << "[CGI][READ] total output bytes=" << cgiOutput.size()
			  << std::endl;
	if (!cgiOutput.empty()) {
		std::string preview(
			cgiOutput.begin(),
			cgiOutput.begin() + std::min((size_t)200, cgiOutput.size()));
		std::cerr << "[CGI][READ] preview=" << preview << std::endl;
	}

	int status;
	waitpid(pid, &status, 0);

	std::cerr << "[CGI][WAIT] raw status=" << status << std::endl;
	std::cerr << "[CGI][WAIT] exited=" << WIFEXITED(status) << std::endl;
	if (WIFEXITED(status))
		std::cerr << "[CGI][WAIT] exit code=" << WEXITSTATUS(status)
				  << std::endl;
	std::cerr << "[CGI][WAIT] signaled=" << WIFSIGNALED(status) << std::endl;
	if (WIFSIGNALED(status))
		std::cerr << "[CGI][WAIT] signal=" << WTERMSIG(status) << std::endl;

	freeEnvp(envp, envpVec.size());

	dup2(saved_stdin, STDIN_FILENO);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdin);
	close(saved_stdout);

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		std::cerr << "[CGI][ERROR] child exit != 0 -> 502" << std::endl;
		return makeResponse(502);
	}

	std::cerr << "[CGI] parseCgiOutput()" << std::endl;
	return parseCgiOutput(cgiOutput);
}