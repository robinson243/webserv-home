/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 14:36:05 by romukena          #+#    #+#             */
/*   Updated: 2026/05/07 00:01:44 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "CgiHandler.hpp"
#include "LocationConfig.hpp"
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

bool isImplementedMethod(const std::string &m) {
	return (m == "GET" || m == "POST" || m == "DELETE");
}

std::string buildAllowHeader(const std::set<std::string> &allow) {
	// Format: "GET, POST, DELETE"
	std::string out;
	for (std::set<std::string>::const_iterator it = allow.begin();
		 it != allow.end();
		 ++it) {
		if (!out.empty())
			out += ", ";
		out += *it;
	}
	return out;
}

HttpResponse makeResponse(int code) {
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
	else if (code == 200)
		body = "";
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
std::set<std::string>
defaultAllowedMethodsIfEmpty(std::set<std::string> allow) {
	if (!allow.empty())
		return allow;
	allow.insert("GET");
	allow.insert("POST");
	allow.insert("DELETE");
	return allow;
}

int findLocation(ServerConfig server, HttpRequest req) {
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	std::vector<LocationConfig>::iterator it;
	std::vector<LocationConfig> loc = server.getLocations();
	int val = -1;
	size_t longestMatch = 0;
	for (it = loc.begin(); it != loc.end(); ++it) {
		std::string path = (*it).getPath();
		if (uri.compare(0, path.size(), path) == 0
			&& (uri.size() == path.size() || uri[path.size()] == '/'
				|| path == "/")
			&& path.size() > longestMatch) {
			longestMatch = path.size();
			val = std::distance(loc.begin(), it);
		}
	}
	return val;
}

std::string concatenatePath(ServerConfig server, HttpRequest req) {
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
	while (p != std::string::npos) {
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
	if (!resolvedPath) {
		// Fichier inexistant : vérification manuelle
		if (finalPath.compare(0, absRoot.size(), absRoot) != 0)
			return "";
		if (finalPath.find("/../") != std::string::npos
			|| finalPath.find("/..") == finalPath.size() - 3)
			return "";
		return finalPath;
	}

	std::string absPath(resolvedPath);
	free(resolvedPath);
	if (absPath.find(absRoot) != 0)
		return "";

	return absPath;
}

bool readFileToString(const std::string &path, std::string &content) {
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return false;
	std::ostringstream ss;
	ss << file.rdbuf();
	content = ss.str();
	return true;
}

std::string getContentType(const std::string &path) {
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

HttpResponse Get(const HttpRequest &req, const ServerConfig &server) {
	struct stat st;
	HttpResponse response;
	int valLocation = findLocation(server, req);
	std::vector<LocationConfig> locations = server.getLocations();

	if (locations.empty() || valLocation == -1) {
		response = makeResponse(404);
		return response;
	}

	const LocationConfig &loc = locations[valLocation];

	if (loc.getCode() >= 300 && loc.getCode() < 400 && !loc.getUrl().empty()) {
		HttpResponse resp;
		resp = makeResponse(loc.getCode());
		resp.addHeadersResponse("Location", loc.getUrl());
		return resp;
	}

	std::vector<std::string> indexes = loc.getIndex();
	if (indexes.empty())
		indexes = server.getIndex();
	std::string path = concatenatePath(server, req);
	if (path.empty()) {
		response = makeResponse(403);
		return response;
	}
	if (stat(path.c_str(), &st) == 0) {
		if (S_ISREG(st.st_mode)) {
			std::string body;
			if (!readFileToString(path, body)) {
				response = makeResponse(403);
				return response;
			}
			std::string contentType = getContentType(path);
			response = makeResponse(200);
			response.addHeadersResponse("Content-Type", contentType);
			std::ostringstream oss;
			oss << body.length();
			response.addHeadersResponse("Content-Length", oss.str());
			response.addBodyResponse(body);
			return response;
		} else if (S_ISDIR(st.st_mode)) {
			std::string uri = req.getRequest().at("uri");
			if (uri.empty() || uri[uri.size() - 1] != '/') {
				response = makeResponse(301);
				response.addHeadersResponse("Location", uri + "/");
				return response;
			}
			// Sous-cas 1 : chercher un fichier index
			for (std::vector<std::string>::iterator it = indexes.begin();
				 it != indexes.end();
				 ++it) {
				std::string indexPath = path + "/" + *it;
				struct stat stIndex;
				if (stat(indexPath.c_str(), &stIndex) == 0
					&& S_ISREG(stIndex.st_mode)) {
					std::string body;
					if (!readFileToString(indexPath, body)) {
						response = makeResponse(403);
						return response;
					}
					std::string contentType = getContentType(indexPath);
					response = makeResponse(200);
					response.addHeadersResponse("Content-Type", contentType);
					std::ostringstream oss;
					oss << body.length();
					response.addHeadersResponse("Content-Length", oss.str());
					response.addBodyResponse(body);
					return response;
				}
			}

			if (locations[valLocation].getAutoindex()) {
				DIR *dir = opendir(path.c_str());
				if (!dir) {
					response = makeResponse(403);
					return response;
				}

				std::string uri = req.getRequest().at("uri");
				std::string html;
				html += "<html><head><title>Index of " + uri
						+ "</title></head><body>";
				html += "<h1>Index of " + uri + "</h1><ul>";

				struct dirent *entry;
				while ((entry = readdir(dir)) != NULL) {
					std::string name = entry->d_name;
					if (name == ".")
						continue;

					std::string fullPath = path + "/" + name;
					struct stat stEntry;
					std::string href = name;
					std::string display = name;

					if (stat(fullPath.c_str(), &stEntry) == 0
						&& S_ISDIR(stEntry.st_mode)) {
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

				response = makeResponse(200);
				response.addHeadersResponse("Content-Type", "text/html");
				response.addHeadersResponse("Content-Length", oss.str());
				response.addBodyResponse(html);
				return response;
			} else {
				response = makeResponse(403);
				return response;
			}
		}
	}
	response = makeResponse(404);
	return response;
}
std::string concatenateLocationPath(const LocationConfig &loc,
									const HttpRequest &req) {
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
	else {
		if (uri.compare(0, locPath.size(), locPath) != 0)
			return "";
		suffix = uri.substr(locPath.size());
		if (suffix.empty())
			suffix = "/";
	}

	std::string finalPath = root;
	if (!finalPath.empty() && finalPath[finalPath.size() - 1] == '/'
		&& !suffix.empty() && suffix[0] == '/')
		finalPath += suffix.substr(1);
	else
		finalPath += suffix;

	size_t p = finalPath.find("//");
	while (p != std::string::npos) {
		finalPath.erase(p, 1);
		p = finalPath.find("//");
	}

	return finalPath;
}

HttpResponse Delete(const HttpRequest &req, const ServerConfig &server) {
	struct stat st;
	HttpResponse response;

	if (!req.getValid()) {
		response = makeResponse(req.getCode());
		return response;
	}

	int valLocation = findLocation(server, req);
	std::vector<LocationConfig> locations = server.getLocations();

	if (locations.empty() || valLocation == -1) {
		response = makeResponse(404);
		return response;
	}

	const LocationConfig &loc = locations[valLocation];
	std::string path = concatenateLocationPath(loc, req);

	if (path.empty()) {
		response = makeResponse(403);
		return response;
	}

	if (stat(path.c_str(), &st) == -1) {
		response = makeResponse(404);
		return response;
	}

	if (S_ISDIR(st.st_mode)) {
		response = makeResponse(403);
		return response;
	}

	if (remove(path.c_str()) == 0) {
		response = makeResponse(204);
		return response;
	}

	response = makeResponse(403);
	return response;
}

HttpResponse Post(const HttpRequest &req, const ServerConfig &server) {
	HttpResponse response;
	int valLocation = findLocation(server, req);
	std::vector<LocationConfig> locations = server.getLocations();

	if (locations.empty() || valLocation == -1) {
		response = makeResponse(404);
		return response;
	}

	size_t maxBody = server.getBodySizeClient();
	size_t contentLength = req.getBody().size();
	if (maxBody != 0 && contentLength > maxBody)
		return makeResponse(413);
	const LocationConfig &loc = locations[valLocation];

	if (loc.hasRedirect()) {
		HttpResponse resp;
		resp = makeResponse(loc.getCode());
		resp.addHeadersResponse("Location", loc.getUrl());
		return resp;
	}

	std::vector<unsigned char> body = req.getBody();
	std::map<std::string, std::string> headers = req.getHeaders();

	if (body.empty()) {
		response = makeResponse(400);
		return response;
	}

	bool hasContentLength = (headers.find("Content-Length") != headers.end());
	bool isChunked = false;
	std::map<std::string, std::string>::iterator itTE =
		headers.find("Transfer-Encoding");
	if (itTE != headers.end() && itTE->second == "chunked")
		isChunked = true;

	if (!hasContentLength && !isChunked) {
		response = makeResponse(400);
		return response;
	}

	if (loc.gethasmaxsize() && loc.getMaxBody() < body.size()) {
		response = makeResponse(413);
		return response;
	}

	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	if (uri.empty()) {
		response = makeResponse(400);
		return response;
	}

	std::string filename;
	size_t p = uri.find_last_of('/');
	if (p == std::string::npos)
		filename = uri;
	else
		filename = uri.substr(p + 1);

	if (filename.empty()) {
		response = makeResponse(400);
		return response;
	}
	if (filename.find("..") != std::string::npos) {
		response = makeResponse(403);
		return response;
	}

	std::string baseDir =
		loc.getUploadPath().empty() ? loc.getRoot() : loc.getUploadPath();
	if (baseDir.empty()) {
		response = makeResponse(500);
		return response;
	}

	std::string path = baseDir + "/" + filename;

	std::ofstream file(path.c_str(), std::ios::binary);
	if (!file.is_open()) {
		response = makeResponse(403);
		return response;
	}

	file.write(reinterpret_cast<const char *>(&body[0]), body.size());
	if (file.fail()) {
		file.close();
		response = makeResponse(500);
		return response;
	}
	file.close();
	response = makeResponse(201);
	return response;
}

// void fillDefaultErrorBody(HttpResponse &resp) {
// 	int code = resp.getCode();
// 	// Simple pages
// 	std::ostringstream html;
// 	html << "<html><head><title>" << code << "</title></head>"
// 		 << "<body><h1>" << code << "</h1></body></html>";
// 	std::string body = html.str();

// 	resp.addHeadersResponse("Content-Type", "text/html");
// 	std::ostringstream len;
// 	len << body.size();
// 	resp.addHeadersResponse("Content-Length", len.str());
// 	resp.addBodyResponse(body);
// }

ServerConfig selectServer(const std::vector<ServerConfig> &servers,
						  const HttpRequest &req) {
	std::map<std::string, std::string> headers = req.getHeaders();
	std::map<std::string, std::string>::const_iterator it =
		headers.find("Host");

	if (it == headers.end())
		return servers[0];

	std::string host = it->second;
	size_t pos = host.find(':');
	if (pos != std::string::npos)
		host = host.substr(0, pos);

	for (std::vector<ServerConfig>::const_iterator sit = servers.begin();
		 sit != servers.end();
		 ++sit) {
		std::vector<std::string> names = sit->getServerName();
		for (size_t i = 0; i < names.size(); i++) {
			if (names[i] == host)
				return (*sit);
		}
	}
	return servers[0];
}

// Politique choisie : si allow_methods est vide sur une location,
// on autorise par défaut les 3 méthodes mandatory : GET, POST, DELETE.
HttpResponse handleRequest(const HttpRequest &req,
						   const std::vector<ServerConfig> &servers) {
	HttpResponse response;
	ServerConfig server = selectServer(servers, req);
	if (!req.getValid()) {
		response = makeResponse(req.getCode());
		return response;
	}

	int valLocation = findLocation(server, req);
	if (valLocation == -1) {
		response = makeResponse(404);
		return response;
	}

	const std::vector<LocationConfig> &locations = server.getLocations();
	const LocationConfig &loc = locations[valLocation];

	if (loc.getCode() >= 300 && loc.getCode() < 400 && !loc.getUrl().empty()) {
		HttpResponse resp = makeResponse(loc.getCode());
		resp.addHeadersResponse("Location", loc.getUrl());
		return resp;
	}

	const std::map<std::string, std::string> &r = req.getRequest();
	std::map<std::string, std::string>::const_iterator it = r.find("method");
	if (it == r.end()) {
		response = makeResponse(400);
		return response;
	}

	const std::string &method = it->second;
	std::set<std::string> allowMeth =
		defaultAllowedMethodsIfEmpty(loc.getAllowMethods());

	if (!isImplementedMethod(method)) {
		response = makeResponse(501);
		return response;
	}

	if (allowMeth.find(method) == allowMeth.end()) {
		response = makeResponse(405);
		response.addHeadersResponse("Allow", buildAllowHeader(allowMeth));
		return response;
	}

	std::string uri = req.getRequest().at("uri");
	size_t qpos = uri.find("?");
	std::string path = (qpos == std::string::npos) ? uri : uri.substr(0, qpos);
	size_t dotpos = path.rfind(".");
	if (dotpos != std::string::npos) {
		std::string ext = path.substr(dotpos);
		const std::map<std::string, std::string> &cgiExt =
			loc.getCgiExtension();
		if (cgiExt.find(ext) != cgiExt.end())
			return handleCgi(req, loc, ext);
	}

	if (method == "GET")
		response = Get(req, server);
	else if (method == "POST")
		response = Post(req, server);
	else if (method == "DELETE")
		response = Delete(req, server);
	else
		response = makeResponse(501);

	return response;
}