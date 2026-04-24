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
#include <cstring>

int findLocation(ServerConfig server, HttpRequest req) {
	std::vector<LocationConfig>::iterator it;
	std::map<std::string, std::string> r = req.getRequest();
	std::string uri = r["uri"];
	int i = 0;
	if (uri.empty())
		return -1;
	for (it = server.getLocations().begin(); it != server.getLocations().end();
		 ++it) {
		if (std::strncmp((*it).getUrl().c_str(), uri.c_str(), uri.length())
			== 0) {
			std::cout << "location url :" << (*it).getUrl() << std::endl;
		}
		i++;
	}
	return i ;
}

HttpResponse Get(HttpRequest req, ServerConfig server) {
}
