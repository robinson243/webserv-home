/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 14:36:05 by romukena          #+#    #+#             */
/*   Updated: 2026/04/25 13:09:08 by romukena         ###   ########.fr       */
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

std::string readFileToString(const std::string &path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		return ""; // Gérer l'erreur d'ouverture
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
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

HttpResponse Get(HttpRequest req, ServerConfig server)
{
	struct stat st;
	int valLocation = findLocation(server, req);
	std::string path = concatenatePath(server, req);
	HttpResponse response;
	if (stat(path.c_str(), &st) == 0)
	{
		if (S_ISREG(st.st_mode))
		{
			std::string body = readFileToString(path);
			std::string contentType = getContentType(path);

			response.addCode(200);
			response.addHeadersResponse("Content-Type", contentType);
			response.addHeadersResponse("Content-Length", std::to_string(body.length()));
			response.addBodyResponse(body);
			return response;
		}
		else if (S_ISDIR(st.st_mode))
		{
		}
	}
	response.addCode(404);
	return (response);
}
