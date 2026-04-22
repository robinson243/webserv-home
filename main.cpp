/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 19:34:17 by ydembele          #+#    #+#             */
/*   Updated: 2026/04/21 19:43:32 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

ServerConfig selectServer(int port, std::string host, std::vector<ServerConfig> &servers)
{
    std::map<int, std::vector<ServerConfig>> serversByPort = groupServersByPort(servers);
    std::map<int, std::vector<ServerConfig>>::iterator it = serversByPort.find(port);
    if (it == serversByPort.end() || it->second.empty())
        return servers[0];
    std::vector<ServerConfig> &s = it->second;
    for (size_t i = 0; i < s.size(); i++)
    {
        for (size_t j = 0; j < s[i].getServerName().size(); j++)
        {
            if (host == s[i].getServerName()[j])
            {
                std::cout << "knlnlk\n";
                return s[i];
            }
        }
    }
    return s[0];
}

int main()
{
    try
    {
        std::vector<ServerConfig> servers = pars("exemple.conf");
        for (size_t i = 0; i < servers.size(); i++)
	        std::cout << servers[i];
        std::map<int, std::vector<ServerConfig>> serversByPort = groupServersByPort(servers);
        
        std::cout << servers.size() << std::endl;
        ServerConfig tmp = selectServer(80, "example.com", servers);
        std::cout << tmp.getRoot();

    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}