/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_server.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 19:34:17 by ydembele          #+#    #+#             */
/*   Updated: 2026/04/25 18:02:40 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

ServerConfig selectServer(int port, std::string host, std::vector<ServerConfig> &servers)
{
    std::map<int, std::vector<ServerConfig*>> serversByPort = groupServersByPort(servers);
    std::map<int, std::vector<ServerConfig*>>::iterator it = serversByPort.find(port);
    if (it == serversByPort.end() || it->second.empty())
        return servers[0];
    std::vector<ServerConfig*> &s = it->second;
    for (size_t i = 0; i < s.size(); i++)
    {
        for (size_t j = 0; j < s[i]->getServerName().size(); j++)
        {
            if (host == s[i]->getServerName()[j])
                return *s[i];
        }
    }
    return *s[0];
}

LocationConfig selectLocation(std::string uri, ServerConfig &servers)
{
    std::vector<LocationConfig> locations = servers.getLocations();
    LocationConfig *l = NULL;
    if (locations.empty())
    {
        l->setPath("/");
        l->setRoot(servers.getRoot());
        l->setIndex(servers.getIndex());
        l->setMaxBody(servers.getBodySizeClient());
        return *l;
    }
    for (size_t i = 0; i < locations.size(); i++)
    {
        std::string path = locations[i].getPath();
        if (uri.compare(0, path.size(), path) == 0)
        {
            if (!l || path.size() > l->getPath().size())
                l = &locations[i];
        }
    }
    if (!l)
        return locations[0];
    return *l;
}

int main(int ac, char **av)
{
    try
    {
        std::vector<ServerConfig> servers;
        if (ac == 1)
            servers = pars("exemple.conf");
        else
            servers = pars(av[1]);
        // for (size_t i = 0; i < servers.size(); i++)
	    //     std::cout << servers[i];
        std::map<int, std::vector<ServerConfig*>> serversByPort = groupServersByPort(servers);
        
        std::cout << servers.size() << std::endl;
        ServerConfig tmp = selectServer(90, "example.com", servers);
        std::cout << tmp;
        LocationConfig ll = selectLocation("/images", tmp);
        std::cout << "\n\n\n";
        std::cout << ll;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}