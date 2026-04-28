/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 14:36:05 by romukena          #+#    #+#             */
/*   Updated: 2026/04/28 00:30:31 by romukena         ###   ########.fr       */
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

int findLocation(ServerConfig server, HttpRequest req) {
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	std::vector<LocationConfig>::iterator it;
	std::vector<LocationConfig> loc = server.getLocations();
	int val = 0;
	for (it = loc.begin(); it != loc.end(); ++it) {
		std::string path = (*it).getPath();
		if (uri.compare(0, path.size(), path) == 0
			&& (uri.size() == path.size() || uri[path.size()] == '/')) {
			val = std::distance(loc.begin(), it);
		}
	}
	return val;
}

std::string concatenatePath(ServerConfig server, HttpRequest req) {
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
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
	char *resolvedPath = realpath(finalPath.c_str(), NULL);
	if (!resolvedPath)
		return finalPath;

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
	if (locations.empty()) {
		response.addCode(404);
		return response;
	}
	std::vector<std::string> indexes = locations[valLocation].getIndex();
	std::string path = concatenatePath(server, req);
	if (stat(path.c_str(), &st) == 0) {
		if (S_ISREG(st.st_mode)) {
			std::string body;
			if (!readFileToString(path, body)) {
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
		} else if (S_ISDIR(st.st_mode)) {
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

			if (locations[valLocation].getAutoindex()) {
				DIR *dir = opendir(path.c_str());
				if (!dir) {
					response.addCode(403);
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

				response.addCode(200);
				response.addHeadersResponse("Content-Type", "text/html");
				response.addHeadersResponse("Content-Length", oss.str());
				response.addBodyResponse(html);
				return response;
			} else {
				response.addCode(403);
				return response;
			}
		}
	}
	response.addCode(404);
	return response;
}

HttpResponse Delete(const HttpRequest &req, const ServerConfig &server) {
	struct stat st;
	HttpResponse response;
	int valLocation = findLocation(server, req);
	std::vector<LocationConfig> locations = server.getLocations();
	if (locations.empty()) {
		response.addCode(404);
		return response;
	}
	if (!locations[valLocation].isMethodAllowed("DELETE")) {
		response.addCode(405);
		return response;
	}
	std::string path = concatenatePath(server, req);
	if (path == "") {
		response.addCode(403);
		return response;
	}
	if (stat(path.c_str(), &st) == -1) {
		response.addCode(404);
		return response;
	}
	if (S_ISDIR(st.st_mode)) {
		response.addCode(403);
		return response;
	}
	if (remove(path.c_str()) == 0) {
		response.addCode(204);
		return response;
	} else {
		response.addCode(403);
		return response;
	}
}

HttpResponse Post(const HttpRequest &req, const ServerConfig &server) {
	struct stat st;
	HttpResponse response;
	int valLocation = findLocation(server, req);
	std::vector<LocationConfig> locations = server.getLocations();
	std::vector<unsigned char> body = req.getBody();

	if (locations.empty()) {
		response.addCode(404);
		return response;
	}
	if (!locations[valLocation].isMethodAllowed("POST")) {
		response.addCode(405);
		return response;
	}
	if (locations[valLocation].gethasmaxsize()
		&& locations[valLocation].getMaxBody() < body.size()) {
		response.addCode(413);
		return response;
	}
	if (locations[valLocation].getUploadPath().empty()) {
		response.addCode(500);
		return response;
	}

	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	if (uri.empty()) {
		response.addCode(400);
		return response;
	}
	std::string URI;
	size_t p = uri.find_last_of("/");
	if (p == std::string::npos)
		URI = uri;
	else
		URI = uri.substr(p + 1);
	if (URI.empty()) {
		response.addCode(400);
		return response;
	}
	if (URI.find("..") != std::string::npos) {
		response.addCode(403);
		return response;
	}
	std::string uploadPath = locations[valLocation].getUploadPath();
	std::string path = uploadPath + "/" + URI;
	std::ofstream file(path, std::ios::binary);
	if (!file.is_open()) {
		response.addCode(403);
		return response;
	}
	std::string str(req.getBody().begin(), req.getBody().end());
	file << str;
	file.close();
	response.addCode(201);
	return response;
}
