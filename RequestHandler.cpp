/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 14:36:05 by romukena          #+#    #+#             */
/*   Updated: 2026/04/26 12:01:17 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "LocationConfig.hpp"
#include <algorithm>
#include <cstring>
#include <iterator>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int findLocation(ServerConfig server, HttpRequest req)
{
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	std::vector<LocationConfig>::iterator it;
	std::vector<LocationConfig> loc = server.getLocations();
	int val = 0;
	for (it = loc.begin(); it != loc.end(); ++it)
	{
		std::string path = (*it).getPath();
		if (uri.compare(0, path.size(), path) == 0 && (uri.size() == path.size() || uri[path.size()] == '/'))
		{
			val = std::distance(loc.begin(), it);
		}
	}
	return val;
}

std::string concatenatePath(ServerConfig server, HttpRequest req)
{
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	std::string root = server.getRoot();
	std::string finalPath = root + uri;
	if (finalPath.find("//") != std::string::npos)
	{
		int pos = finalPath.find("//");
		finalPath.erase(pos, 1);
	}
	return finalPath;
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
	std::vector<LocationConfig> locations = server.getLocations();
	std::vector<std::string> indexes = locations[valLocation].getIndex();
	std::string path = concatenatePath(server, req);
	if (locations.empty())
	{
		response.addCode(404);
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
			// Sous-cas 1 : chercher un fichier index
			for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); ++it)
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
			// Sous-cas 2 : autoindex
			if (locations[valLocation].getAutoindex())
			{
				// TODO : générer le listing HTML du dossier
			}
			else
			{
				// Sous-cas 3 : ni index ni autoindex → 403
				response.addCode(403);
				return response;
			}
		}
	}
	response.addCode(404);
	return response;
}