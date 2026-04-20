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
#include <fstream>
#include <array>
#include <string>

ServerConfig::ServerConfig()
{
	_root = "";
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
	_data = tokenize(data);
}

std::vector<ServerConfig> pars(const std::string &file)
{
	std::string content = LoadConfigFile(file);

	if (content.empty())
    throw std::runtime_error("Config file is empty or cannot be read");
	
	std::vector<ServerConfig> servers;
	std::vector<std::string> tokens = tokenize(content);

	std::vector<std::string>::const_iterator it = tokens.begin();

	while (it != tokens.end())
	{
    if (*it != "server")
        throw std::runtime_error("Expected 'server'");
    servers.push_back(parseServer(it, tokens.end()));
		++it;
	}
	return servers;
}

ServerConfig parseServer(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	if (it == end || *it != "server")
    throw std::runtime_error("Expected 'server'");
	++it;
	if (it == end || *it != "{")
    throw std::runtime_error("Expected '{' after server");
	++it;
	ServerConfig server;
	try {
		for (; it != end && *it != "}"; it++)
			parseDirective(it, end, server);
		if (it == end)
			throw std::runtime_error("");
		++it;
	}
	catch (const std::exception &e){
		throw;
	}
	return server;
}

void parseDirective(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, ServerConfig &server)
{
	std::array<std::string, 7> directives = {"listen", "server_name", "root", "index", "error_page", "client_max_body_size", "location"};
	int i = 0;
	while (i < 7 && *it != directives[i])
		i++;
	if (i == 7)
		throw std::runtime_error("Unknown directive: " + *it);
	switch (i)
	{
		case 0:
			server.setPort(findPort(it, end));
		case 1:
			server.setServerName(findServerName(it, end));
		case 2:
		{
			std::string tmp = server.getRoot();
			if (!tmp.empty())
				throw std::runtime_error("Multiple definitions of root");
			server.setRoot(findRoot(it, end));
		}
		case 3:
			server.setIndex(findIndex(it, end));
		case 4:
			parseErrorPage(it, end, server);
	}
}

void	parseErrorPage(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, ServerConfig &server)
{
	int code;
	std::string path;

	++it;
	if (it == end)
		throw std::runtime_error("Error page: missing value");
	code = std::stoi(*it);
	++it;
	if (it == end)
		throw std::runtime_error("Error page: missing value");
	if (*it == "}" || *it == "{" || *it == ";")
		throw std::runtime_error("Error page: invalid value");
	path = *it;
	++it;
	if (it == end || *it != ";")
		throw std::runtime_error("Error page: ';'");
	server.setErrorPage(code, path);
}

std::vector<std::string> findIndex(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	std::vector<std::string> index;
	++it;
	if (it == end)
		throw std::runtime_error("Index: missing value");
	while (it != end && *it != ";")
	{
		if (*it == "}" || *it == "{")
			throw std::runtime_error("Index: brace in name forbidden");
		index.push_back(*it);
		++it;
	}
	if (it == end)
		throw std::runtime_error("Index: unterminated directive");
	if (index.empty())
		throw std::runtime_error("Index: no names provided");
	++it; // Skip ;
	return index;
}

std::string findRoot(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	++it;
	if (it == end)
		throw std::runtime_error("Root: missing root");
	if (*it == "}" || *it == "{" || *it == ";")
		throw std::runtime_error("Root: invalid value");
	std::string s = *it;
	++it;
	if (it == end || *it != ";")
		throw std::runtime_error("Root: expected ';'");
	++it;
	return s;
}

unsigned int findPort(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	++it;
	if (it == end)
		throw std::runtime_error("listen: missing port");
	if (*it == "}" || *it == "{")
		throw std::runtime_error("listen: brace in port forbidden");
	int port = std::stoi(*it);
	if (port < 1 || port > 65535)
		throw std::runtime_error("listen: invalid port range [1-65535]");
	++it;
	if (it == end || *it != ";")
    throw std::runtime_error("listen: expected ';'");
	++it; // Skip ;
	return static_cast<unsigned int>(port);
}

std::vector<std::string> findServerName(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	std::vector<std::string> servername;
	++it;
	if (it == end)
		throw std::runtime_error("server_name: missing value");
	while (it != end && *it != ";")
	{
		if (*it == "}" || *it == "{")
			throw std::runtime_error("server_name: brace in name forbidden");
		servername.push_back(*it);
		++it;
	}
	if (it == end)
		throw std::runtime_error("server_name: unterminated directive");
	if (servername.empty())
		throw std::runtime_error("server_name: no names provided");
	++it; // Skip ;
	return servername;
}

std::string	LoadConfigFile(const std::string &file)
{
	std::string	data;
	std::string line;
	std::ifstream f(file);

	if (!f.is_open())
		throw std::runtime_error("Cannot open file");
	while (std::getline(f, line))
		data += line + "\n";
	return data;
}

std::vector<std::string>	tokenize(const std::string &data)
{
	std::string	current;
	std::vector<std::string> token;

	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i] == '#')
		{
			while (i < data.size() && data[i] != '\n')
				i++;
			continue ;
		}
		if (data[i] == '"')
		{
			if (!current.empty())
    	{
        token.push_back(current);
        current.clear();
    	}
    	i++;
			std::string value;
			while (i < data.size() && data[i] != '"')
    	{
        value += data[i];
        i++;
    	}
    	if (i == data.size())
        throw std::runtime_error("Unclosed quote");
			if (value.size() == 1 && (value[0] == '}' || value[0] == '{'))
				throw std::runtime_error("Lone brace in quoted string");
			if (!value.empty())
				token.push_back(value);
			continue;
		}
		if (isspace(data[i]))
		{
			if (!current.empty())
			{
				token.push_back(current);
				current.clear();
			}
			continue;
		}
		if (data[i] == '{' || data[i] == '}' || data[i] == ';')
		{
			if (!current.empty())
			{
				token.push_back(current);
				current.clear();
			}
			token.push_back(std::string(1, data[i]));
      continue;
		}
		current += data[i];
	}
	if (!current.empty())
	{
		token.push_back(current);
		current.clear();
	}
	return token;
}

void	ServerConfig::parsConfig(std::vector<std::string> &data)
{
	if (data.empty())
		return ;
	
}


std::vector<unsigned int> ServerConfig::getPort() const
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

std::vector<std::string> ServerConfig::getIndex() const
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
	_port.push_back(port);
}
void	ServerConfig::setServerName(std::vector<std::string> servername)
{
	_serverName = servername;
}

void	ServerConfig::setRoot(std::string root)
{
	_root = root;
}

void	ServerConfig::setIndex(std::vector<std::string> index)
{
	for (std::vector<std::string>::iterator it = index.begin(); it != index.end(); ++it)
		_index.push_back(*it);
}

void	ServerConfig::setSizeClient(size_t size)
{
	_clientMaxBodySize = size;
}

void	ServerConfig::setErrorPage(int i, std::string s)
{
	_errorPage[i] = s;
}

void	ServerConfig::setLocations(LocationConfig locations)
{
	_locations.push_back(locations);
}
