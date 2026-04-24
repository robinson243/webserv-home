/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 14:36:05 by romukena          #+#    #+#             */
/*   Updated: 2026/04/24 16:45:41 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include "LocationConfig.hpp"
#include <algorithm>
#include <cstring>
#include <iterator>

int findLocation(ServerConfig server, HttpRequest req) {
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	std::vector<LocationConfig>::iterator it;
	std::vector<LocationConfig> loc = server.getLocations();
	int val = 0;
	std::cout << "mon uri: " << uri << std::endl;
	for (it = loc.begin(); it != loc.end(); ++it) {
		std::string path = (*it).getPath();
		if (uri.compare(0, path.size(), path) == 0
			&& (uri.size() == path.size() || uri[path.size()] == '/')) {
			val = std::distance(loc.begin(), it);
		}
	}
	return val;
}

HttpResponse Get(HttpRequest req, ServerConfig server) {
}
