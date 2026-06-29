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
		return makeResponse(502);

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
	std::string REQUEST_METHOD = req.getRequest().at("method");
	std::string interpretor = loc.getCgiExtension().at(ext);
	std::string SCRIPT_NAME;
	std::string scriptPath;
	std::string uri = req.getRequest().at("uri");

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

	if (!loc.getPath().empty()) {
		bool isPrefix =
			SCRIPT_NAME.size() >= loc.getPath().size()
			&& SCRIPT_NAME.compare(0, loc.getPath().size(), loc.getPath()) == 0;

		bool hasValidBoundary = SCRIPT_NAME.size() == loc.getPath().size()
								|| SCRIPT_NAME[loc.getPath().size()] == '/';

		if (isPrefix && hasValidBoundary) {
			std::string relativePath = SCRIPT_NAME.substr(loc.getPath().size());

			if (relativePath.empty())
				relativePath = "/";

			if (!loc.getAlias().empty()) {
				scriptPath = loc.getAlias() + relativePath;
			} else {
				scriptPath = loc.getRoot() + relativePath;
			}
		}
	}

	if (scriptPath.empty()) {
		return makeResponse(500);
	}

	if (access(scriptPath.c_str(), F_OK) != 0) {
		if (errno == EACCES || errno == EPERM)
			return makeResponse(403);
		return makeResponse(404);
	}

	if (access(scriptPath.c_str(), X_OK) != 0) {
		return makeResponse(403);
	}
	std::string CONTENT_TYPE;
	std::map<std::string, std::string>::const_iterator itCt =
		req.getHeaders().find("content-type");
	if (itCt != req.getHeaders().end() && !itCt->second.empty())
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
	if (!CONTENT_TYPE.empty())
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
	}
	envp[envpVec.size()] = NULL;

	char *argv[] = { (char *)interpretor.c_str(),
					 (char *)scriptPath.c_str(),
					 NULL };

	int pipefdIn[2];
	int pipefdOut[2];
	pid_t pid;

	if (pipe(pipefdIn) == -1) {
		return freeEnvp(envp, envpVec.size()), makeResponse(500);
	}

	if (pipe(pipefdOut) == -1) {
		close(pipefdIn[0]);
		close(pipefdIn[1]);
		return freeEnvp(envp, envpVec.size()), makeResponse(500);
	}

	pid = fork();
	if (pid == -1) {
		close(pipefdIn[0]);
		close(pipefdIn[1]);
		close(pipefdOut[0]);
		close(pipefdOut[1]);
		return freeEnvp(envp, envpVec.size()), makeResponse(500);
	}

	if (pid == 0) {
		if (dup2(pipefdIn[0], STDIN_FILENO) == -1)
			_exit(1);
		if (dup2(pipefdOut[1], STDOUT_FILENO) == -1)
			_exit(1);

		close(pipefdIn[0]);
		close(pipefdIn[1]);
		close(pipefdOut[0]);
		close(pipefdOut[1]);

		execve(interpretor.c_str(), argv, envp);

		_exit(1);
	}

	close(pipefdIn[0]);
	close(pipefdOut[1]);

	const std::string &method = req.getRequest().at("method");
	if (method == "POST" && !req.getBody().empty()) {
		const std::vector<unsigned char> &body = req.getBody();

		size_t totalWritten = 0;
		while (totalWritten < body.size()) {
			ssize_t written = write(
				pipefdIn[1],
				reinterpret_cast<const char *>(body.data() + totalWritten),
				body.size() - totalWritten);

			if (written == -1) {
				close(pipefdIn[1]);
				close(pipefdOut[0]);
				kill(pid, SIGKILL);
				waitpid(pid, NULL, 0);

				return freeEnvp(envp, envpVec.size()), makeResponse(500);
			}

			if (written == 0) {
				close(pipefdIn[1]);
				close(pipefdOut[0]);
				kill(pid, SIGKILL);
				waitpid(pid, NULL, 0);

				return freeEnvp(envp, envpVec.size()), makeResponse(500);
			}

			totalWritten += static_cast<size_t>(written);
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
			close(pipefdOut[0]);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			return freeEnvp(envp, envpVec.size()), makeResponse(504);
		}

		n = read(pipefdOut[0], buf, sizeof(buf));
		if (n == -1) {
			close(pipefdOut[0]);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			return freeEnvp(envp, envpVec.size()), makeResponse(500);
		}
		if (n == 0)
			break;

		cgiOutput.insert(cgiOutput.end(), buf, buf + n);
	}

	close(pipefdOut[0]);

	if (!cgiOutput.empty()) {
		std::string preview(
			cgiOutput.begin(),
			cgiOutput.begin() + std::min((size_t)200, cgiOutput.size()));
	}

	int status;
	waitpid(pid, &status, 0);

	freeEnvp(envp, envpVec.size());

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		return makeResponse(502);
	}

	return parseCgiOutput(cgiOutput);
}