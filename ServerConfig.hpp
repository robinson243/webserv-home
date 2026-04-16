/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:01:30 by romukena          #+#    #+#             */
/*   Updated: 2026/04/16 13:39:17 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "LocationConfig.hpp"
#include <climits>
#include <iostream>
#include <map>
#include <set>
#include <vector>

class ServerConfig {
private:
	unsigned int _port;
	std::vector<std::string> _serverName;
	std::string _root;
	std::string _index;
	size_t _clientMaxBodySize;
	std::map<int, std::string> _errorPage;
	std::vector<LocationConfig> _locations;

  public:
	ServerConfig();
	~ServerConfig();
	unsigned int getPort() const;
	
};
