/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 14:36:05 by romukena          #+#    #+#             */
/*   Updated: 2026/04/25 01:47:32 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "LocationConfig.hpp"
#include <algorithm>
#include <cstring>
#include <iterator>
#include <sys/stat.h>

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

HttpResponse Get(HttpRequest req, ServerConfig server)
{
	struct stat st;
	int valLocation = findLocation(server, req);
	std::string path = concatenatePath(server, req);
	std::map<std::string, std::string> header = req.getHeaders();
	std::string contentLength, contentType;
	std::vector<unsigned char> b = req.getBody();
	std::string body(b.begin(), b.end());
	contentType = header["Content-Type"];
	contentLength = header["Content-Length"];
	HttpResponse response;
	if (stat(path.c_str(), &st) == 0)
	{
		if (S_ISREG(st.st_mode))
		{
			response.addCode(200);
			response.addHeadersResponse("Content-Type", contentType);
			response.addHeadersResponse("Content-Length", contentLength);
			response.addBodyResponse(body);
			return response;
		}
		else if (S_ISDIR(st.st_mode))
		{
			/* code */
		}
	}
	response.addCode(404);
	return (response);
}
