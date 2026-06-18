#include "LocationConfig.hpp"
#include "RequestHandler.hpp"
#include "ServerConfig.hpp"

bool isCgiRequest(const HttpRequest &req, const LocationConfig &loc)
{
	std::string uri = req.getRequest().at("uri");
	std::string cgiFile;
	std::string ext;
	size_t pos = uri.find("?");
	if (pos == std::string::npos)
		cgiFile = uri;
	else
		cgiFile = uri.substr(0, pos);

	size_t postExt = cgiFile.find_last_of(".");
	if (postExt != std::string::npos)
	{
		ext = cgiFile.substr(postExt);
		std::map<std::string, std::string>::const_iterator it;
		for (it = loc.getCgiExtension().begin();
			 it != loc.getCgiExtension().end();
			 ++it)
		{
			if (it->first == ext)
				return true;
		}
	}
	return false;
}

HttpResponse parseCgiOutput(const std::vector<unsigned char> &output)
{
	// Convertir en string pour trouver la séparation headers/body
	std::string raw(output.begin(), output.end());

	// Chercher la ligne vide qui sépare headers et body
	std::string sep = "\r\n\r\n";
	size_t sepPos = raw.find(sep);

	// fallback si le CGI utilise \n\n au lieu de \r\n\r\n
	if (sepPos == std::string::npos)
	{
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

	while (std::getline(stream, line))
	{
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
		if (key == "Status")
		{
			// "Status: 200 OK" -> extraire 200
			std::istringstream ss(value);
			ss >> code;
		}
		else
		{
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

void freeEnvp(char **envp, size_t size)
{
	for (size_t i = 0; i < size; ++i)
		delete[] envp[i];
	delete[] envp;
}

HttpResponse handleCgi(const HttpRequest &req,
					   const LocationConfig &loc,
					   const std::string &ext)
{
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
	std::vector<std::string> envpVec;
	envpVec.push_back("REQUEST_METHOD=" + REQUEST_METHOD);
	envpVec.push_back("QUERY_STRING=" + QUERY_STRING);
	envpVec.push_back("CONTENT_LENGTH=" + CONTENT_LENGTH);
	envpVec.push_back("SCRIPT_NAME=" + SCRIPT_NAME);

	if (!loc.getPath().empty())
	{
		bool isPrefix =
			SCRIPT_NAME.size() >= loc.getPath().size() && SCRIPT_NAME.compare(0, loc.getPath().size(), loc.getPath()) == 0;

		bool hasValidBoundary = SCRIPT_NAME.size() == loc.getPath().size() || SCRIPT_NAME[loc.getPath().size()] == '/';
		if (isPrefix && hasValidBoundary)
		{
			if (!loc.getAlias().empty())
			{
				scriptPath += loc.getAlias();
				scriptPath += SCRIPT_NAME.substr(loc.getPath().size());
			}

			else
			{
				scriptPath += loc.getRoot();
				scriptPath += SCRIPT_NAME;
			}
		}
	}
	if (scriptPath.empty())
		return makeResponse(500);
	char **envp = new char *[envpVec.size() + 1];
	for (size_t i = 0; i < envpVec.size(); ++i)
	{
		envp[i] = new char[envpVec[i].size() + 1];
		strcpy(envp[i], envpVec[i].c_str());
	}
	envp[envpVec.size()] = NULL;

	char *argv[] = {(char *)interpretor.c_str(),
					(char *)scriptPath.c_str(),
					NULL};
	int pipefdIn[2];
	int pipefdOut[2];
	pid_t pid;

	if (pipe(pipefdIn) == -1)
		return freeEnvp(envp, envpVec.size()), makeResponse(500);

	if (pipe(pipefdOut) == -1)
	{
		close(pipefdIn[0]);
		close(pipefdIn[1]);
		return freeEnvp(envp, envpVec.size()), makeResponse(500);
	}

	pid = fork();
	if (pid == -1)
	{
		close(pipefdIn[0]);
		close(pipefdIn[1]);
		close(pipefdOut[0]);
		close(pipefdOut[1]);
		return freeEnvp(envp, envpVec.size()), makeResponse(500);
	}
	int saved_stdin = dup(STDIN_FILENO);
	int saved_stdout = dup(STDOUT_FILENO);

	if (pid == 0)
	{
		if (dup2(pipefdIn[0], STDIN_FILENO) == -1)
		{
			close(pipefdIn[0]);
			close(pipefdIn[1]);
			close(pipefdOut[0]);
			close(pipefdOut[1]);
			_exit(1);
		}

		if (dup2(pipefdOut[1], STDOUT_FILENO) == -1)
		{
			close(pipefdIn[0]);
			close(pipefdIn[1]);
			close(pipefdOut[0]);
			close(pipefdOut[1]);
			_exit(1);
		}

		close(pipefdIn[0]);
		close(pipefdIn[1]);
		close(pipefdOut[0]);
		close(pipefdOut[1]);

		execve(scriptPath.c_str(), argv, envp);
		_exit(1);
	}

	close(pipefdIn[0]);
	close(pipefdOut[1]);
	/* Parent */
	const std::string &method = req.getRequest().at("method");
	if (method == "POST" && !req.getBody().empty())
	{
		const std::vector<unsigned char> &body = req.getBody();
		ssize_t written = write(pipefdIn[1],
								reinterpret_cast<const char *>(body.data()),
								body.size());
		if (written == -1)
		{
			close(pipefdIn[1]);
			close(pipefdOut[0]);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			dup2(saved_stdin, STDIN_FILENO);   // ← manquait
			dup2(saved_stdout, STDOUT_FILENO); // ← manquait
			close(saved_stdin);				   // ← manquait
			close(saved_stdout);			   // ← manquait
			return freeEnvp(envp, envpVec.size()), makeResponse(500);
		}
	}
	close(pipefdIn[1]);

	std::vector<unsigned char> cgiOutput;
	char buf[4096];
	ssize_t n;
	time_t start = time(NULL);
	const int CGI_TIMEOUT = 5;

	while (true)
	{
		if (time(NULL) - start > CGI_TIMEOUT)
		{
			close(pipefdOut[0]);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			dup2(saved_stdin, STDIN_FILENO);   // ← manquait
			dup2(saved_stdout, STDOUT_FILENO); // ← manquait
			close(saved_stdin);				   // ← manquait
			close(saved_stdout);			   // ← manquait
			return freeEnvp(envp, envpVec.size()), makeResponse(504);
		}

		n = read(pipefdOut[0], buf, sizeof(buf));
		if (n == -1)
		{
			close(pipefdOut[0]);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, 0);
			dup2(saved_stdin, STDIN_FILENO);   // ← manquait
			dup2(saved_stdout, STDOUT_FILENO); // ← manquait
			close(saved_stdin);				   // ← manquait
			close(saved_stdout);			   // ← manquait
			return freeEnvp(envp, envpVec.size()), makeResponse(500);
		}
		if (n == 0)
			break;
		cgiOutput.insert(cgiOutput.end(), buf, buf + n);
	}

	close(pipefdOut[0]);

	int status;
	waitpid(pid, &status, 0);

	freeEnvp(envp, envpVec.size());

	dup2(saved_stdin, STDIN_FILENO);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdin);
	close(saved_stdout);

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		return makeResponse(502);
	return parseCgiOutput(cgiOutput);
}
