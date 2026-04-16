/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 13:40:39 by ydembele          #+#    #+#             */
/*   Updated: 2026/04/16 14:43:17 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

#include <limits>

ServerConfig::ServerConfig()
{
	_port = 0;
	_root = "";
	_index = "";
	_clientMaxBodySize = std::numeric_limits<std::size_t>::max();
}

ServerConfig &ServerConfig::operator=(ServerConfig &other)
{
	if (this != &other)
	{
		_port = other._port;
		_serverName = other._serverName;
		_root = other._root;
		_index = other._index;
		_clientMaxBodySize = other._clientMaxBodySize;
		_errorPage = other._errorPage;
		_locations = other._locations;
	}
	return *this;
}

ServerConfig::~ServerConfig()
{}

ServerConfig::ServerConfig(const std::string data)
{
	
}


std::string	ServerConfig::recupLine(const std::string search, std::string data)
{
	std::string line;
	size_t i = data.find(search);
	if (i == std::string::npos)
		return (0);
	if (i + search.length() - data.length() <= 0)
		return (0);
	line = data.substr(i + search.length(), data.length());
	return line;
}

unsigned int ServerConfig::getPort() const
{
	return _port;
}

std::vector<std::string> ServerConfig::getServerName() const
{
	return _serverName;
}

std::string ServerConfig::getRoot() const
{
	return _root;
}

std::string ServerConfig::getIndex() const
{
	return _index;
}

size_t ServerConfig::getBodySizeClient() const
{
	return _clientMaxBodySize;
}

std::map<int, std::string> ServerConfig::getErrorPage() const
{
	return _errorPage;
}

std::vector<LocationConfig> ServerConfig::getLocations() const
{
	return _locations;
}


void	ServerConfig::setPort(unsigned int port)
{
	_port = port;
}
void	ServerConfig::setServerName(std::vector<std::string> servername)
{
	_serverName = servername;
}

void	ServerConfig::setRoot(std::string root)
{
	_root = root;
}

void	ServerConfig::setIndex(std::string index)
{
	_index = index;
}

void	ServerConfig::setSizeClient(size_t size)
{
	_clientMaxBodySize = size;
}

void	ServerConfig::setErrorPage(std::map<int, std::string> errorpage)
{
	_errorPage = errorpage;
}

void	ServerConfig::setLocations(std::vector<LocationConfig> locations)
{
	_locations = locations;
}
