/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_server.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 19:34:17 by ydembele          #+#    #+#             */
/*   Updated: 2026/05/01 14:51:21 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include "ServerConfig.hpp"


int main(int ac, char **av)
{
    try
    {
        std::vector<ServerConfig> servers;

        if (ac == 1)
            servers = pars("exemple.conf");
        else
            servers = pars(av[1]);

        std::cout << "=== SERVERS PARSED ===" << std::endl;
        for (size_t i = 0; i < servers.size(); i++)
            std::cout << servers[i] << std::endl;

        std::cout << "\n=== GROUPING TEST ===" << std::endl;
        std::map<int, std::vector<ServerConfig*> > serversByPort = groupServersByPort(servers);
        std::cout << "serversByPort size = " << serversByPort.size() << std::endl;

        std::cout << "\n=== SELECT SERVER TEST ===" << std::endl;
        ServerConfig *tmp = selectServer(90, "example.com", servers);
        std::cout << *tmp << std::endl;

        // std::cout << "\n=== SELECT LOCATION TEST ===" << std::endl;
        // LocationConfig ll = selectLocation("/images", *tmp);
        // std::cout << ll << std::endl;

        // std::cout << "\n=== BUILD SOCKETS TEST ===" << std::endl;
        // std::vector<ListenSocket> sockets = buildListenSockets(servers);

        // std::cout << "Sockets created: " << sockets.size() << std::endl;

        // for (size_t i = 0; i < sockets.size(); i++)
        // {
        //     std::cout << "Socket " << i << std::endl;
        //     std::cout << "  host: " << sockets[i].host << std::endl;
        //     std::cout << "  port: " << sockets[i].port << std::endl;
        //     std::cout << "  fd: " << sockets[i].fd << std::endl;
        //     std::cout << "  servers linked: " << sockets[i].servers.size() << std::endl;
        // }

        // std::cout << "\n=== TEST ACCEPT READY ===" << std::endl;
        // std::cout << "Server is ready (no crash = good)" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
