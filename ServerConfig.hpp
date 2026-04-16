/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:01:30 by romukena          #+#    #+#             */
/*   Updated: 2026/04/16 14:30:34 by ydembele         ###   ########.fr       */
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
	ServerConfig(const std::string data);
	ServerConfig &operator=(ServerConfig &other);
	~ServerConfig();

	std::string	ServerConfig::recupLine(const std::string search, std::string data);

	unsigned int getPort() const;
	std::vector<std::string> getServerName() const;
	std::string getRoot() const;
	std::string getIndex() const;
	size_t getBodySizeClient() const;
	std::map<int, std::string> getErrorPage() const;
	std::vector<LocationConfig> getLocations() const;

	void	setPort(unsigned int port);
	void	setServerName(std::vector<std::string> Servername);
	void	setRoot(std::string root);
	void	setIndex(std::string index);
	void	setSizeClient(size_t size);
	void	setErrorPage(std::map<int, std::string> errorpage);
	void	setLocations(std::vector<LocationConfig> locations);
};
