/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 19:34:17 by ydembele          #+#    #+#             */
/*   Updated: 2026/04/20 21:12:29 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

int main()
{
    try
    {
        std::vector<ServerConfig> servers = pars("exemple.conf");
        for (size_t i = 0; i < servers.size(); i++)
	        std::cout << servers[i];
        std::cout << servers.size() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}