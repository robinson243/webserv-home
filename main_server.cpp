/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_server.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 19:34:17 by ydembele          #+#    #+#             */
/*   Updated: 2026/04/27 19:18:37 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

ServerConfig *selectServer(int port, std::string host, std::vector<ServerConfig> &servers)
{
    std::map<int, std::vector<ServerConfig*>> serversByPort = groupServersByPort(servers);
    std::map<int, std::vector<ServerConfig*>>::iterator it = serversByPort.find(port);
    if (it == serversByPort.end() || it->second.empty())
        return &servers[0];
    std::vector<ServerConfig*> &s = it->second;
    for (size_t i = 0; i < s.size(); i++)
    {
        for (size_t j = 0; j < s[i]->getServerName().size(); j++)
        {
            if (host == s[i]->getServerName()[j])
                return s[i];
        }
    }
    return s[0];
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

std::vector<ListenSocket> buildListenSockets(std::vector<ServerConfig> &servers)
{
    std::vector<ListenSocket> result;

    for (size_t i = 0; i < servers.size(); i++)
    {
        ServerConfig &srv = servers[i];
        std::vector<unsigned int> ports = srv.getPort();
        std::vector<std::string> hosts = srv.getListenHosts();

        std::cout << ports.size() << " " << hosts.size() << std::endl << std::endl;
        for (size_t p = 0; p < ports.size(); p++)
        {
            std::string host = "0.0.0.0";
            if (p < hosts.size())
                host = hosts[p];
            else if (!hosts.empty())
                host = hosts[0];
            unsigned int port = ports[p];
            bool found = false;

            for (size_t j = 0; j < result.size(); j++)
            {
                if (result[j].host == host && result[j].port == port)
                {
                    result[j].servers.push_back(&srv);
                    found = true;
                    break;
                }
            }
            if (found)
                continue;

            ListenSocket ls;

            ls.host = host;
            ls.port = port;
            ls.servers.push_back(&srv);
            ls.fd = socket(AF_INET, SOCK_STREAM, 0);
            if (ls.fd < 0)
                throw std::runtime_error("socket failed");
            int opt = 1;
            setsockopt(ls.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            ls.addr.sin_family = AF_INET;
            ls.addr.sin_port = htons(port);
            ls.addr.sin_addr.s_addr = inet_addr(host.c_str());
            if (bind(ls.fd, (sockaddr*)&ls.addr, sizeof(ls.addr)) < 0)
                throw std::runtime_error("bind failed");
            if (listen(ls.fd, 128) < 0)
                throw std::runtime_error("listen failed");
            result.push_back(ls);
        }
    }
    return result;
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
        
        // std::cout << servers.size() << std::endl;
        ServerConfig *tmp = selectServer(90, "example.com", servers);
        // std::cout << *tmp;
        LocationConfig ll = selectLocation("/images", *tmp);
        std::cout << "\n\n\n";
        // std::cout << ll;
        std::vector<ListenSocket> s = buildListenSockets(servers);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
