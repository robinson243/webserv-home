/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 14:36:05 by romukena          #+#    #+#             */
/*   Updated: 2026/06/16 16:26:18 by oamairi          ###   ########.fr       */
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
	return (m == "GET" || m == "POST" || m == "DELETE" || m == "HEAD");
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

	if (code == 100)
		body = "<html><body><h1>100 Continue</h1></body></html>";
	else if (code == 101)
		body = "<html><body><h1>101 Switching Protocols</h1></body></html>";

	// 2xx Success
	else if (code == 200)
		body = "<html><body><h1>200 OK</h1></body></html>";
	else if (code == 201)
		body = "<html><body><h1>201 Created</h1></body></html>";
	else if (code == 202)
		body = "<html><body><h1>202 Accepted</h1></body></html>";
	else if (code == 204)
		body = "<html><body><h1>204 No Content</h1></body></html>";

	// 3xx Redirection
	else if (code == 301)
		body = "<html><body><h1>301 Moved Permanently</h1></body></html>";
	else if (code == 302)
		body = "<html><body><h1>302 Found</h1></body></html>";
	else if (code == 303)
		body = "<html><body><h1>303 See Other</h1></body></html>";
	else if (code == 304)
		body = "<html><body><h1>304 Not Modified</h1></body></html>";
	else if (code == 307)
		body = "<html><body><h1>307 Temporary Redirect</h1></body></html>";
	else if (code == 308)
		body = "<html><body><h1>308 Permanent Redirect</h1></body></html>";

	// 4xx Client Errors
	else if (code == 400)
		body = "<html><body><h1>400 Bad Request</h1></body></html>";
	else if (code == 401)
		body = "<html><body><h1>401 Unauthorized</h1></body></html>";
	else if (code == 403)
		body = "<html><body><h1>403 Forbidden</h1></body></html>";
	else if (code == 404)
		body = "<html><body><h1>404 Not Found</h1></body></html>";
	else if (code == 405)
		body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
	else if (code == 408)
		body = "<html><body><h1>408 Request Timeout</h1></body></html>";
	else if (code == 409)
		body = "<html><body><h1>409 Conflict</h1></body></html>";
	else if (code == 410)
		body = "<html><body><h1>410 Gone</h1></body></html>";
	else if (code == 411)
		body = "<html><body><h1>411 Length Required</h1></body></html>";
	else if (code == 413)
		body = "<html><body><h1>413 Payload Too Large</h1></body></html>";
	else if (code == 414)
		body = "<html><body><h1>414 URI Too Long</h1></body></html>";
	else if (code == 415)
		body = "<html><body><h1>415 Unsupported Media Type</h1></body></html>";
	else if (code == 429)
		body = "<html><body><h1>429 Too Many Requests</h1></body></html>";

	// 5xx Server Errors
	else if (code == 500)
		body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
	else if (code == 501)
		body = "<html><body><h1>501 Not Implemented</h1></body></html>";
	else if (code == 502)
		body = "<html><body><h1>502 Bad Gateway</h1></body></html>";
	else if (code == 503)
		body = "<html><body><h1>503 Service Unavailable</h1></body></html>";
	else if (code == 504)
		body = "<html><body><h1>504 Gateway Timeout</h1></body></html>";
	else if (code == 505)
		body =
			"<html><body><h1>505 HTTP Version Not Supported</h1></body></html>";

	r.addCode(code);

	if (!body.empty()) {
		r.addHeadersResponse("content-type", "text/html");
		std::ostringstream ss;
		ss << body.size();
		r.addHeadersResponse("content-length", ss.str());
		r.setBody(std::vector<unsigned char>(body.begin(), body.end()));
	}

	return r;
}

// Choix "sujet-friendly": si allowed_methods est vide, on autorise au moins
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

int findLocation(const ServerConfig &server, const HttpRequest &req) {
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	std::vector<LocationConfig>::iterator it;
	std::vector<LocationConfig> loc = server.getLocations();
	int val = -1;
	size_t longestMatch = 0;

	for (it = loc.begin(); it != loc.end(); ++it) {
		std::string path = (*it).getPath();

		// Normalise : retire le slash final du path pour la comparaison
		std::string normPath = path;
		if (normPath.size() > 1 && normPath[normPath.size() - 1] == '/')
			normPath = normPath.substr(0, normPath.size() - 1);

		if (uri.compare(0, normPath.size(), normPath) == 0
			&& (uri.size() == normPath.size() || uri[normPath.size()] == '/'
				|| normPath == "" || path == "/")
			&& normPath.size() > longestMatch) {
			longestMatch = normPath.size();
			val = std::distance(loc.begin(), it);
		}
	}
	return val;
}

std::string concatenatePath(const ServerConfig &server,
							const HttpRequest &req) {
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
		resp.addHeadersResponse("location", loc.getUrl());
		return resp;
	}

	std::vector<std::string> indexes = loc.getIndex();
	if (indexes.empty())
		indexes = server.getIndex();
	std::string path;
	if (!loc.getRoot().empty())
		path = concatenateLocationPath(loc, req);
	else
		path = concatenatePath(server, req);
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
			response.addHeadersResponse("content-type", contentType);
			std::ostringstream oss;
			oss << body.length();
			response.addHeadersResponse("content-length", oss.str());
			response.setBody(
				std::vector<unsigned char>(body.begin(), body.end()));
			return response;
		} else if (S_ISDIR(st.st_mode)) {
			std::string uri = req.getRequest().at("uri");
			if (uri.empty() || uri[uri.size() - 1] != '/') {
				response = makeResponse(301);
				response.addHeadersResponse("location", uri + "/");
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
					response.addHeadersResponse("content-type", contentType);
					std::ostringstream oss;
					oss << body.length();
					response.addHeadersResponse("content-length", oss.str());
					response.setBody(
						std::vector<unsigned char>(body.begin(), body.end()));
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
				response.addHeadersResponse("content-type", "text/html");
				response.addHeadersResponse("content-length", oss.str());
				response.setBody(
					std::vector<unsigned char>(html.begin(), html.end()));
				return response;
			} else {
				response = makeResponse(404);
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

	const LocationConfig &loc = locations[valLocation];

	size_t effectiveMaxBody = server.getBodySizeClient();
	if (loc.gethasmaxsize())
		effectiveMaxBody = loc.getMaxBody();

	std::map<std::string, std::string> headers = req.getHeaders();

	if (effectiveMaxBody != 0) {
		std::map<std::string, std::string>::iterator it =
			headers.find("content-length");

		if (it != headers.end()) {
			std::istringstream iss(it->second);
			size_t announcedSize = 0;
			iss >> announcedSize;
			if (!iss.fail() && announcedSize > effectiveMaxBody)
				return makeResponse(413);
		}

		if (req.getBody().size() > effectiveMaxBody)
			return makeResponse(413);
	}

	if (loc.hasRedirect()) {
		HttpResponse resp;
		resp = makeResponse(loc.getCode());
		resp.addHeadersResponse("location", loc.getUrl());
		return resp;
	}

	std::vector<unsigned char> body = req.getBody();

	if (body.empty()) {
		response = makeResponse(400);
		return response;
	}

	bool hasContentLength = (headers.find("content-length") != headers.end());
	bool isChunked = false;
	std::map<std::string, std::string>::iterator itTE =
		headers.find("transfer-encoding");
	if (itTE != headers.end() && itTE->second == "chunked")
		isChunked = true;

	if (!hasContentLength && !isChunked) {
		response = makeResponse(400);
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

	if (filename.find("..") != std::string::npos
		|| filename.find("%2e%2e") != std::string::npos
		|| filename.find("%2E%2E") != std::string::npos
		|| filename.find("%2e%2E") != std::string::npos
		|| filename.find("%2E%2e") != std::string::npos) {
		response = makeResponse(403);
		return response;
	}

	std::string baseDir =
		loc.getUploadPath().empty() ? loc.getRoot() : loc.getUploadPath();
	if (baseDir.empty()) {
		response = makeResponse(500);
		return response;
	}

	std::string path;
	if (!baseDir.empty() && baseDir[baseDir.size() - 1] == '/')
		path = baseDir + filename;
	else
		path = baseDir + "/" + filename;

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

const ServerConfig &selectServer(const std::vector<ServerConfig> &servers,
								 const HttpRequest &req) {
	std::map<std::string, std::string> headers = req.getHeaders();
	std::map<std::string, std::string>::const_iterator it =
		headers.find("host");

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

// Politique choisie : si allowed_methods est vide sur une location,
// on autorise par défaut les 3 méthodes mandatory : GET, POST, DELETE.

HttpResponse handleRequest(const HttpRequest &req,
						   const std::vector<ServerConfig> &servers) {
	HttpResponse response;

	std::cerr << "\n========== handleRequest ==========" << std::endl;
	std::cerr << "[REQ] valid=" << req.getValid() << std::endl;

	const std::map<std::string, std::string> &r = req.getRequest();
	std::map<std::string, std::string>::const_iterator itMethodDbg =
		r.find("method");
	std::map<std::string, std::string>::const_iterator itUriDbg = r.find("uri");

	if (itMethodDbg != r.end())
		std::cerr << "[REQ] method=" << itMethodDbg->second << std::endl;
	else
		std::cerr << "[REQ] method=<missing>" << std::endl;

	if (itUriDbg != r.end())
		std::cerr << "[REQ] uri=" << itUriDbg->second << std::endl;
	else
		std::cerr << "[REQ] uri=<missing>" << std::endl;

	std::map<std::string, std::string>::const_iterator itCl =
		r.find("content-length");
	if (itCl != r.end())
		std::cerr << "[REQ] content-length=" << itCl->second << std::endl;
	else
		std::cerr << "[REQ] content-length=<missing>" << std::endl;

	const ServerConfig &server = selectServer(servers, req);
	std::cerr << "[ROUTE] server selected" << std::endl;

	if (!req.getValid()) {
		std::cerr << "[ERROR] invalid request, code=" << req.getCode()
				  << std::endl;
		response = makeResponse(req.getCode());
		std::cerr << "[RESP] status=" << response.getCode() << std::endl;
		return response;
	}

	int valLocation = findLocation(server, req);
	std::cerr << "[ROUTE] findLocation=" << valLocation << std::endl;
	if (valLocation == -1) {
		std::cerr << "[ERROR] no matching location -> 404" << std::endl;
		response = makeResponse(404);
		std::cerr << "[RESP] status=" << response.getCode() << std::endl;
		return response;
	}

	const std::vector<LocationConfig> &locations = server.getLocations();
	const LocationConfig &loc = locations[valLocation];
	std::cerr << "[ROUTE] location matched index=" << valLocation << std::endl;

	if (loc.getCode() >= 300 && loc.getCode() < 400 && !loc.getUrl().empty()) {
		std::cerr << "[ROUTE] redirect code=" << loc.getCode()
				  << " url=" << loc.getUrl() << std::endl;
		HttpResponse resp = makeResponse(loc.getCode());
		resp.addHeadersResponse("location", loc.getUrl());
		std::cerr << "[RESP] status=" << resp.getCode() << std::endl;
		return resp;
	}

	std::map<std::string, std::string>::const_iterator it = r.find("method");
	if (it == r.end()) {
		std::cerr << "[ERROR] method missing -> 400" << std::endl;
		response = makeResponse(400);
		std::cerr << "[RESP] status=" << response.getCode() << std::endl;
		return response;
	}

	const std::string &method = it->second;
	std::set<std::string> allowMeth =
		defaultAllowedMethodsIfEmpty(loc.getAllowMethods());

	std::cerr << "[REQ] method final=" << method << std::endl;

	if (!isImplementedMethod(method)) {
		std::cerr << "[ERROR] method not implemented -> 501" << std::endl;
		response = makeResponse(501);
		std::cerr << "[RESP] status=" << response.getCode() << std::endl;
		return response;
	}

	std::string uri = req.getRequest().at("uri");
	size_t qpos = uri.find("?");
	std::string path = (qpos == std::string::npos) ? uri : uri.substr(0, qpos);
	size_t dotpos = path.rfind(".");

	std::cerr << "[REQ] path=" << path << std::endl;

	std::cerr << "[METHOD] allowed=";
	for (std::set<std::string>::iterator mit = allowMeth.begin();
		 mit != allowMeth.end();
		 ++mit)
		std::cerr << *mit << " ";
	std::cerr << std::endl;

	if (allowMeth.find(method) == allowMeth.end()) {
		std::cerr << "[ERROR] method not allowed -> 405" << std::endl;
		response = makeResponse(405);
		response.addHeadersResponse("allow", buildAllowHeader(allowMeth));
		std::cerr << "[RESP] status=" << response.getCode() << std::endl;
		return response;
	}

	if (dotpos != std::string::npos) {
		std::string ext = path.substr(dotpos);
		const std::map<std::string, std::string> &cgiExt =
			loc.getCgiExtension();

		std::cerr << "[CGI] ext=" << ext << std::endl;
		std::cerr << "[CGI] configured=" << (cgiExt.find(ext) != cgiExt.end())
				  << std::endl;

		if (cgiExt.find(ext) != cgiExt.end() && method == "POST") {
			std::cerr << "[CGI] POST CGI branch" << std::endl;

			size_t bodySize = req.getBody().size();
			size_t maxBodySize = 1000000;

			std::cerr << "[POST-CGI-CHECK] uri=" << path << " content_length="
					  << (itCl != r.end() ? itCl->second : "<missing>")
					  << " body_size=" << req.getBody().size()
					  << " max=" << maxBodySize << std::endl;

			if (bodySize > maxBodySize) {
				std::cerr << "[ERROR] body too large -> 413" << std::endl;
				HttpResponse resp = makeResponse(413);
				std::cerr << "[RESP] status=" << resp.getCode() << std::endl;
				return resp;
			}

			HttpResponse cgiResp = handleCgi(req, loc, ext);
			std::cerr << "[RESP] status=" << cgiResp.getCode() << std::endl;
			return cgiResp;
		}

		if (cgiExt.find(ext) != cgiExt.end() && method == "GET") {
			std::cerr << "[CGI] GET CGI branch" << std::endl;
			HttpResponse cgiResp = handleCgi(req, loc, ext);
			std::cerr << "[RESP] status=" << cgiResp.getCode() << std::endl;
			return cgiResp;
		}
	}

	if (method == "GET") {
		std::cerr << "[DISPATCH] GET" << std::endl;
		response = Get(req, server);
	} else if (method == "HEAD") {
		std::cerr << "[DISPATCH] HEAD" << std::endl;
		response = Get(req, server);
		response.setBody(std::vector<unsigned char>());
		response.addHeadersResponse("connection", "close");
	} else if (method == "POST") {
		std::cerr << "[DISPATCH] POST" << std::endl;
		response = Post(req, server);
	} else if (method == "DELETE") {
		std::cerr << "[DISPATCH] DELETE" << std::endl;
		response = Delete(req, server);
	} else {
		std::cerr << "[ERROR] fallback 501" << std::endl;
		response = makeResponse(501);
	}

	std::cerr << "[RESP] final status=" << response.getCode() << std::endl;
	return response;
}