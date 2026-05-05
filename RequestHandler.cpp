/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 14:36:05 by romukena          #+#    #+#             */
/*   Updated: 2026/05/06 01:44:31 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "LocationConfig.hpp"
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>

static bool isImplementedMethod(const std::string &m)
{
	return (m == "GET" || m == "POST" || m == "DELETE");
}

static std::string buildAllowHeader(const std::set<std::string> &allow)
{
	// Format: "GET, POST, DELETE"
	std::string out;
	for (std::set<std::string>::const_iterator it = allow.begin();
		 it != allow.end();
		 ++it)
	{
		if (!out.empty())
			out += ", ";
		out += *it;
	}
	return out;
}

static HttpResponse makeRedirectResponse(int code, const std::string &url)
{
	HttpResponse r;
	r.addCode(code);
	r.addHeadersResponse("Location", url);
	r.addHeadersResponse("Content-Length", "0");
	return r;
}

static HttpResponse makeErrorResponse(int code)
{
	HttpResponse r;
	std::string body;

	if (code == 400)
		body = "<html><body><h1>400 Bad Request</h1></body></html>";
	else if (code == 403)
		body = "<html><body><h1>403 Forbidden</h1></body></html>";
	else if (code == 404)
		body = "<html><body><h1>404 Not Found</h1></body></html>";
	else if (code == 405)
		body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
	else if (code == 413)
		body = "<html><body><h1>413 Payload Too Large</h1></body></html>";
	else if (code == 500)
		body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
	else if (code == 502)
		body = "<html><body><h1>502 Bad Gateway</h1></body></html>";
	else if (code == 504)
		body = "<html><body><h1>504 Gateway Timeout</h1></body></html>";
	else
		body = "<html><body><h1>Error</h1></body></html>";

	r.addCode(code);
	r.addHeadersResponse("Content-Type", "text/html");
	std::ostringstream ss;
	ss << body.size();
	r.addHeadersResponse("Content-Length", ss.str());
	r.setBody(std::vector<unsigned char>(body.begin(), body.end()));
	return r;
}

// Choix "sujet-friendly": si allow_methods est vide, on autorise au moins
// GET/POST/DELETE.
static std::set<std::string>
defaultAllowedMethodsIfEmpty(std::set<std::string> allow)
{
	if (!allow.empty())
		return allow;
	allow.insert("GET");
	allow.insert("POST");
	allow.insert("DELETE");
	return allow;
}

int findLocation(ServerConfig server, HttpRequest req)
{
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	std::vector<LocationConfig>::iterator it;
	std::vector<LocationConfig> loc = server.getLocations();
	int val = -1;
	size_t longestMatch = 0;
	for (it = loc.begin(); it != loc.end(); ++it)
	{
		std::string path = (*it).getPath();
		if (uri.compare(0, path.size(), path) == 0 && (uri.size() == path.size() || uri[path.size()] == '/' || path == "/") && path.size() > longestMatch)
		{
			longestMatch = path.size();
			val = std::distance(loc.begin(), it);
		}
	}
	return val;
}

std::string concatenatePath(ServerConfig server, HttpRequest req)
{
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	size_t q = uri.find('?');
	if (q != std::string::npos)
		uri = uri.substr(0, q);
	size_t h = uri.find('#');
	if (h != std::string::npos)
		uri = uri.substr(0, h);
	if (uri.find("/..") != std::string::npos)
		return "";
	std::string root = server.getRoot();
	std::string finalPath = root + uri;

	size_t p = finalPath.find("//");
	while (p != std::string::npos)
	{
		finalPath.erase(p, 1);
		p = finalPath.find("//");
	}

	char *resolvedRoot = realpath(root.c_str(), NULL);
	if (!resolvedRoot)
		return "";
	std::string absRoot(resolvedRoot);
	free(resolvedRoot);
	finalPath = absRoot + uri;
	char *resolvedPath = realpath(finalPath.c_str(), NULL);
	if (!resolvedPath)
	{
		if (finalPath.find(absRoot) != 0)
			return "";
		return finalPath;
	}

	std::string absPath(resolvedPath);
	free(resolvedPath);
	if (absPath.find(absRoot) != 0)
		return "";

	return absPath;
}

bool readFileToString(const std::string &path, std::string &content)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return false;
	std::ostringstream ss;
	ss << file.rdbuf();
	content = ss.str();
	return true;
}

std::string getContentType(const std::string &path)
{
	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return "text/plain";

	std::string ext = path.substr(pos);

	if (ext == ".html" || ext == ".htm")
		return "text/html";
	if (ext == ".css")
		return "text/css";
	if (ext == ".js")
		return "application/javascript";
	if (ext == ".jpg" || ext == ".jpeg")
		return "image/jpeg";
	if (ext == ".png")
		return "image/png";
	if (ext == ".gif")
		return "image/gif";
	if (ext == ".txt")
		return "text/plain";
	if (ext == ".json")
		return "application/json";
	if (ext == ".pdf")
		return "application/pdf";

	return "application/octet-stream";
}

HttpResponse Get(const HttpRequest &req, const ServerConfig &server)
{
	struct stat st;
	HttpResponse response;
	int valLocation = findLocation(server, req);
	int locIndex = findLocation(server, req);
	std::vector<LocationConfig> locations = server.getLocations();

	if (locations.empty() || locIndex == -1)
	{
		response.addCode(404);
		return response;
	}

	const LocationConfig &loc = locations[locIndex];

	if (loc.getCode() >= 300 && loc.getCode() < 400 && !loc.getUrl().empty())
	{
		HttpResponse resp;
		resp.addCode(loc.getCode());
		resp.addHeadersResponse("Location", loc.getUrl());
		return resp;
	}

	std::vector<std::string> indexes = loc.getIndex();
	if (indexes.empty())
		indexes = server.getIndex();
	std::string path = concatenatePath(server, req);
	if (path.empty())
	{
		response.addCode(403);
		return response;
	}
	if (stat(path.c_str(), &st) == 0)
	{
		if (S_ISREG(st.st_mode))
		{
			std::string body;
			if (!readFileToString(path, body))
			{
				response.addCode(403);
				return response;
			}
			std::string contentType = getContentType(path);
			response.addCode(200);
			response.addHeadersResponse("Content-Type", contentType);
			std::ostringstream oss;
			oss << body.length();
			response.addHeadersResponse("Content-Length", oss.str());
			response.addBodyResponse(body);
			return response;
		}
		else if (S_ISDIR(st.st_mode))
		{
			std::string uri = req.getRequest().at("uri");
			if (uri.empty() || uri[uri.size() - 1] != '/')
			{
				response.addCode(301);
				response.addHeadersResponse("Location", uri + "/");
				return response;
			}
			// Sous-cas 1 : chercher un fichier index
			for (std::vector<std::string>::iterator it = indexes.begin();
				 it != indexes.end();
				 ++it)
			{
				std::string indexPath = path + "/" + *it;
				struct stat stIndex;
				if (stat(indexPath.c_str(), &stIndex) == 0 && S_ISREG(stIndex.st_mode))
				{
					std::string body;
					if (!readFileToString(indexPath, body))
					{
						response.addCode(403);
						return response;
					}
					std::string contentType = getContentType(indexPath);
					response.addCode(200);
					response.addHeadersResponse("Content-Type", contentType);
					std::ostringstream oss;
					oss << body.length();
					response.addHeadersResponse("Content-Length", oss.str());
					response.addBodyResponse(body);
					return response;
				}
			}

			if (locations[valLocation].getAutoindex())
			{
				DIR *dir = opendir(path.c_str());
				if (!dir)
				{
					response.addCode(403);
					return response;
				}

				std::string uri = req.getRequest().at("uri");
				std::string html;
				html += "<html><head><title>Index of " + uri + "</title></head><body>";
				html += "<h1>Index of " + uri + "</h1><ul>";

				struct dirent *entry;
				while ((entry = readdir(dir)) != NULL)
				{
					std::string name = entry->d_name;
					if (name == ".")
						continue;

					std::string fullPath = path + "/" + name;
					struct stat stEntry;
					std::string href = name;
					std::string display = name;

					if (stat(fullPath.c_str(), &stEntry) == 0 && S_ISDIR(stEntry.st_mode))
					{
						href = name + "/";
						display = name + "/";
					}

					html +=
						"<li><a href=\"" + href + "\">" + display + "</a></li>";
				}
				closedir(dir);

				html += "</ul></body></html>";

				std::ostringstream oss;
				oss << html.length();

				response.addCode(200);
				response.addHeadersResponse("Content-Type", "text/html");
				response.addHeadersResponse("Content-Length", oss.str());
				response.addBodyResponse(html);
				return response;
			}
			else
			{
				response.addCode(403);
				return response;
			}
		}
	}
	response.addCode(404);
	return response;
}
static std::string concatenateLocationPath(const LocationConfig &loc,
										   const HttpRequest &req)
{
	std::string uri = req.getRequest().at("uri");

	size_t q = uri.find('?');
	if (q != std::string::npos)
		uri = uri.substr(0, q);

	size_t h = uri.find('#');
	if (h != std::string::npos)
		uri = uri.substr(0, h);

	if (uri.empty())
		return "";

	if (uri.find("/..") != std::string::npos)
		return "";

	std::string root = loc.getRoot();
	if (root.empty())
		return "";

	std::string locPath = loc.getPath();
	std::string suffix;

	if (locPath == "/")
		suffix = uri;
	else
	{
		if (uri.compare(0, locPath.size(), locPath) != 0)
			return "";
		suffix = uri.substr(locPath.size());
		if (suffix.empty())
			suffix = "/";
	}

	std::string finalPath = root;
	if (!finalPath.empty() && finalPath[finalPath.size() - 1] == '/' && !suffix.empty() && suffix[0] == '/')
		finalPath += suffix.substr(1);
	else
		finalPath += suffix;

	size_t p = finalPath.find("//");
	while (p != std::string::npos)
	{
		finalPath.erase(p, 1);
		p = finalPath.find("//");
	}

	return finalPath;
}

HttpResponse Delete(const HttpRequest &req, const ServerConfig &server)
{
	struct stat st;
	HttpResponse response;

	if (!req.getValid())
	{
		response.addCode(req.getCode());
		return response;
	}

	int valLocation = findLocation(server, req);
	std::vector<LocationConfig> locations = server.getLocations();

	if (locations.empty() || valLocation == -1)
	{
		response.addCode(404);
		return response;
	}

	const LocationConfig &loc = locations[valLocation];
	std::string path = concatenateLocationPath(loc, req);

	if (path.empty())
	{
		response.addCode(403);
		return response;
	}

	if (stat(path.c_str(), &st) == -1)
	{
		response.addCode(404);
		return response;
	}

	if (S_ISDIR(st.st_mode))
	{
		response.addCode(403);
		return response;
	}

	if (remove(path.c_str()) == 0)
	{
		response.addCode(204);
		return response;
	}

	response.addCode(403);
	return response;
}

HttpResponse Post(const HttpRequest &req, const ServerConfig &server)
{
	HttpResponse response;
	int valLocation = findLocation(server, req);
	std::vector<LocationConfig> locations = server.getLocations();

	if (locations.empty() || valLocation == -1)
	{
		response.addCode(404);
		return response;
	}

	const LocationConfig &loc = locations[valLocation];

	if (loc.hasRedirect())
	{
		HttpResponse resp;
		resp.addCode(loc.getCode());
		resp.addHeadersResponse("Location", loc.getUrl());
		return resp;
	}

	std::vector<unsigned char> body = req.getBody();
	std::map<std::string, std::string> headers = req.getHeaders();

	if (body.empty())
	{
		response.addCode(400);
		return response;
	}

	bool hasContentLength = (headers.find("Content-Length") != headers.end());
	bool isChunked = false;
	std::map<std::string, std::string>::iterator itTE =
		headers.find("Transfer-Encoding");
	if (itTE != headers.end() && itTE->second == "chunked")
		isChunked = true;

	if (!hasContentLength && !isChunked)
	{
		response.addCode(400);
		return response;
	}

	if (loc.gethasmaxsize() && loc.getMaxBody() < body.size())
	{
		response.addCode(413);
		return response;
	}

	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	if (uri.empty())
	{
		response.addCode(400);
		return response;
	}

	std::string filename;
	size_t p = uri.find_last_of('/');
	if (p == std::string::npos)
		filename = uri;
	else
		filename = uri.substr(p + 1);

	if (filename.empty() || filename.find("..") != std::string::npos)
	{
		response.addCode(403);
		return response;
	}

	std::string baseDir =
		loc.getUploadPath().empty() ? loc.getRoot() : loc.getUploadPath();
	if (baseDir.empty())
	{
		response.addCode(500);
		return response;
	}

	std::string path = baseDir + "/" + filename;

	std::ofstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		response.addCode(403);
		return response;
	}

	file.write(reinterpret_cast<const char *>(&body[0]), body.size());
	file.close();

	response.addCode(201);
	return response;
}

static void fillDefaultErrorBody(HttpResponse &resp)
{
	int code = resp.getCode();
	// Simple pages
	std::ostringstream html;
	html << "<html><head><title>" << code << "</title></head>"
		 << "<body><h1>" << code << "</h1></body></html>";
	std::string body = html.str();

	resp.addHeadersResponse("Content-Type", "text/html");
	std::ostringstream len;
	len << body.size();
	resp.addHeadersResponse("Content-Length", len.str());
	resp.addBodyResponse(body);
}

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
	envpVec.push_back("CONTENT_LENGTH=" + std::to_string(req.getBody().size()));
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
	char **envp = new char *[envpVec.size() + 1];
	for (size_t i = 0; i < envpVec.size(); ++i)
	{
		envp[i] = new char[envpVec[i].size() + 1];
		strcpy(envp[i], envpVec[i].c_str());
	}
	envp[envpVec.size()] = NULL;

	char *argv[] = {(char *)interpretor.c_str(),
					(char *)SCRIPT_NAME.c_str(),
					NULL};
	int pipefdIn[2];
	int pipefdOut[2];
	pid_t pid;

	if (pipe(pipefdIn) == -1)
		return makeErrorResponse(500);

	if (pipe(pipefdOut) == -1)
	{
		close(pipefdIn[0]);
		close(pipefdIn[1]);
		return makeErrorResponse(500);
	}

	pid = fork();
	if (pid == -1)
	{
		close(pipefdIn[0]);
		close(pipefdIn[1]);
		close(pipefdOut[0]);
		close(pipefdOut[1]);
		return makeErrorResponse(500);
	}

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

	// le parent continue ici
	// pipefdIn[1]  -> écrire body CGI
	// pipefdOut[0] -> lire sortie CGI

	// for (size_t i = 0; i < envpVec.size(); ++i)
	// 	delete[] envp[i];
	// delete[] envp;
}

// Politique choisie : si allow_methods est vide sur une location,
// on autorise par défaut les 3 méthodes mandatory : GET, POST, DELETE.
HttpResponse handleRequest(const HttpRequest &req, const ServerConfig &server)
{
	HttpResponse response;

	if (!req.getValid())
	{
		response.addCode(req.getCode());
		fillDefaultErrorBody(response);
		return response;
	}

	int valLocation = findLocation(server, req);
	if (valLocation == -1)
	{
		response.addCode(404);
		fillDefaultErrorBody(response);
		return response;
	}

	const std::vector<LocationConfig> &locations = server.getLocations();
	const LocationConfig &loc = locations[valLocation];

	if (loc.getCode() >= 300 && loc.getCode() < 400 && !loc.getUrl().empty())
		return makeRedirectResponse(loc.getCode(), loc.getUrl());

	const std::map<std::string, std::string> &r = req.getRequest();
	std::map<std::string, std::string>::const_iterator it = r.find("method");
	if (it == r.end())
	{
		response.addCode(400);
		fillDefaultErrorBody(response);
		return response;
	}

	const std::string &method = it->second;
	std::set<std::string> allowMeth =
		defaultAllowedMethodsIfEmpty(loc.getAllowMethods());

	if (!isImplementedMethod(method))
	{
		response.addCode(501);
		fillDefaultErrorBody(response);
		return response;
	}

	if (allowMeth.find(method) == allowMeth.end())
	{
		response.addCode(405);
		response.addHeadersResponse("Allow", buildAllowHeader(allowMeth));
		fillDefaultErrorBody(response);
		return response;
	}

	if (method == "GET")
		response = Get(req, server);
	else if (method == "POST")
		response = Post(req, server);
	else if (method == "DELETE")
		response = Delete(req, server);
	else
		response.addCode(501);

	if (response.getCode() >= 400)
		fillDefaultErrorBody(response);

	return response;
}