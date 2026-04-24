/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 13:40:39 by ydembele          #+#    #+#             */
/*   Updated: 2026/04/24 17:00:49 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

#include <limits>
#include <fstream>
#include <array>
#include <string>
#include "LocationConfig.hpp"

ServerConfig::ServerConfig()
{
	_root = "";
	_clientMaxBodySize = std::numeric_limits<std::size_t>::max();
	_hasMaxSize = false;
}

ServerConfig &ServerConfig::operator=(const ServerConfig &other)
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

ServerConfig::ServerConfig(const ServerConfig &other)
{
    _port = other._port;
    _serverName = other._serverName;
    _root = other._root;
    _index = other._index;
    _clientMaxBodySize = other._clientMaxBodySize;
    _errorPage = other._errorPage;
    _locations = other._locations;
}

ServerConfig::~ServerConfig()
{}


std::vector<ServerConfig> pars(const std::string &file)
{
	std::string content = LoadConfigFile(file);

	if (content.empty())
    throw std::runtime_error("Config file is empty or cannot be read");
	
	std::vector<ServerConfig> servers;
	std::vector<Token> tokens = tokenize(content);

	std::vector<Token>::iterator it = tokens.begin();
	if (it == tokens.end() || it->value != "server")
    throw std::runtime_error("Config must start with server");
	while (it != tokens.end())
	{
    if (it->value != "server")
        throw std::runtime_error("Expected 'server' " + it->value);
		ServerConfig server = parseServer(it, tokens.end());
		validateServer(server);
    servers.push_back(server);
	}
	if (servers.empty())
		throw std::runtime_error("Servers empty");
	return servers;
}

std::map<int, std::vector<ServerConfig*>> groupServersByPort(const std::vector<ServerConfig> &servers)
{
	std::map<int, std::vector<ServerConfig*>> serversByPort;
	for (size_t i = 0; i < servers.size(); i++)
	{
		const std::vector<unsigned int> port = servers[i].getPort();
		for (size_t j = 0; j < port.size(); j++)
			serversByPort[port[j]].push_back(const_cast<ServerConfig*>(&servers[i]));
	}
	return (serversByPort);
}

void validateServer(ServerConfig &server)
{
	if (server.getPort().empty())
		throw std::runtime_error("Server: missing listen");
	if (server.getIndex().empty())
	{
		std::vector<std::string> def;
		def.push_back("index.html");
		server.setIndex(def);
	}
	std::set<std::string> paths;
	std::vector<LocationConfig> &locations = server.getLocations();
	for (size_t i = 0; i < locations.size(); i++)
	{
		const std::string &path = locations[i].getPath();
		if (path.empty() || path[0] != '/')
			throw std::runtime_error("Location: invalid path");
		if (paths.count(path))
			throw std::runtime_error("Location: duplicate path: " + path);
		paths.insert(path);
	}
	for (size_t i = 0; i < locations.size(); i++)
	{
		LocationConfig &loc = locations[i];
		if (loc.getRoot().empty())
			loc.setRoot(server.getRoot());
		if (loc.getIndex().empty())
			loc.setIndex(server.getIndex());
		if (loc.gethasmaxsize() == 0 && server.getHasMaxSize())
			loc.setMaxBody(server.getBodySizeClient());
		
	}
}

ServerConfig parseServer(std::vector<Token>::iterator &it, std::vector<Token>::iterator end)
{
	if (it == end || it->value != "server")
    throw std::runtime_error("Expected 'server'");
	++it;
	if (it == end || it->value != "{" ||  it->in_quotes)
    throw std::runtime_error("Expected '{' after server");
	++it;
	ServerConfig server;
	for (; it != end && (it->value != "}" || it->in_quotes);)
		parseDirective(it, end, server);
	if (it == end)
		throw std::runtime_error("Server not close");
	++it;
	return server;
}

void parseDirective(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, ServerConfig &server)
{
	std::array<std::string, 7> directives = {"listen", "server_name", "root", "index", "error_page", "client_max_body_size", "location"};
	int i = 0;
	while (i < 7 && it->value != directives[i])
		i++;
	if (i == 7)
		throw std::runtime_error("Unknown directive: " + it->value);
	std::cout << it->value << std::endl;
	switch (i)
	{
		case 0:
			server.setPort(findPort(it, end));
			break;
		case 1:
		{
			if (!server.getServerName().empty())
				throw std::runtime_error("Multiple definitions of server_name");
			server.setServerName(findServerName(it, end));
			break;
		}
		case 2:
		{
			if (!server.getRoot().empty())
				throw std::runtime_error("Multiple definitions of root");
			server.setRoot(findRoot(it, end));
			break;
		}
		case 3:
			server.setIndex(findIndex(it, end));
			break;
		case 4:
			parseErrorPage(it, end, server);
			break;
		case 5:
		{
			if (server.getHasMaxSize() == true)
				throw std::runtime_error("Multiple definitions of client_max_body_size");
			server.setSizeClient(findSize(it, end));
			server.setHasMaxSize(true);
			break;
		}
		case 6:
			server.setLocations(parseLocation(it, end));
			break;
	}
}

bool isNumber(const std::string& s)
{
    if (s.empty())
        return false;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    }
    return true;
}

size_t findSize(std::vector<Token>::iterator &it, std::vector<Token>::iterator end)
{
	++it;
	if (it == end)
		throw std::runtime_error("client_max_body_size: missing value");
	if (!isNumber(it->value))
    throw std::runtime_error("client_max_body_size: Invalid body size");
	size_t value = std::strtoul((it->value).c_str(), NULL, 10);
	if (value == 0)
    throw std::runtime_error("client_max_body_size: Body size must be > 0");
	++it;
	if (it == end || *it != ";" || it->in_quotes)
		throw std::runtime_error("client_max_body_size: Body size: missing ';");
	++it;
	return value;
}

void	parseErrorPage(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, ServerConfig &server)
{
	int code;
	std::string path;

	++it;
	if (it == end)
		throw std::runtime_error("Error page: missing value");
	try
	{
		code = std::stoi(it->value);
	}
	catch (...)
	{
		throw std::runtime_error("Error page: invalid code format");
	}
	if (code < 100 || code > 599)
		throw std::runtime_error("Error page: invalid code range [100-599]");
	++it;
	if (it == end)
		throw std::runtime_error("Error page: missing value");
	if ((*it == "}" || *it == "{" || *it == ";") && !it->in_quotes)
		throw std::runtime_error("Error page: invalid value");
	path = it->value;
	++it;
	if (it == end || *it != ";" || it->in_quotes)
		throw std::runtime_error("Error page: ';'");
	++it;
	server.setErrorPage(code, path);
}

std::vector<std::string> findIndex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end)
{
	std::vector<std::string> index;
	++it;
	if (it == end)
		throw std::runtime_error("Index: missing value");
	while (it != end)
	{
		if (*it == ";" && !it->in_quotes)
			break ;
		if ((*it == "}" || *it == "{") && it->in_quotes)
			throw std::runtime_error("Index: brace in name forbidden");
		index.push_back(it->value);
		++it;
	}
	if (it == end)
		throw std::runtime_error("Index: unterminated directive");
	if (index.empty())
		throw std::runtime_error("Index: no names provided");
	++it; // Skip ;
	return index;
}

std::string findRoot(std::vector<Token>::iterator &it, std::vector<Token>::iterator end)
{
	++it;
	if (it == end)
		throw std::runtime_error("Root: missing root");
	if ((*it == "}" || *it == "{" || *it == ";") && !it->in_quotes)
		throw std::runtime_error("Root: invalid value");
	std::string s = it->value;
	++it;
	if (it == end || *it != ";" || it->in_quotes)
		throw std::runtime_error("Root: expected ';'");
	++it;
	return s;
}

unsigned int findPort(std::vector<Token>::iterator &it, std::vector<Token>::iterator end)
{
	++it;
	if (it == end)
		throw std::runtime_error("listen: missing port");
	if ((*it == "}" && !it->in_quotes) || (*it == "{" && !it->in_quotes))
		throw std::runtime_error("listen: brace in port forbidden");
	int port = 0;
	try
	{
		port = std::stoi(it->value);
	}
	catch (...)
	{
		throw std::runtime_error("listen: invalid port format");
	}
	if (port < 1 || port > 65535)
		throw std::runtime_error("listen: invalid port range [1-65535]");
	++it;
	if (it == end || *it != ";" || it->in_quotes)
    throw std::runtime_error("listen: expected ';'");
	++it; // Skip ;
	return static_cast<unsigned int>(port);
}

std::vector<std::string> findServerName(std::vector<Token>::iterator &it, std::vector<Token>::iterator end)
{
	std::vector<std::string> servername;
	++it;
	if (it == end)
		throw std::runtime_error("server_name: missing value");
	while (it != end && !(*it == ";" && !it->in_quotes))
	{
		if ((*it == "}" || *it == "{") && !it->in_quotes)
			throw std::runtime_error("server_name: brace in name forbidden");
		servername.push_back(it->value);
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

std::vector<Token>	tokenize(const std::string &data)
{
	std::string	current;
	std::vector<Token> tokens;

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
        tokens.push_back(current);
        current.clear();
    	}
    	i++;
			std::string value;
			while (i < data.size())
			{
				if (data[i] == '\\' && i + 1 < data.size())
				{
					value += data[i + 1];
					i += 2;
				}
				else if (data[i] == '"')
					break;
				else
				{
					value += data[i];
					i++;
				}
			}
    	if (i == data.size())
        throw std::runtime_error("Unclosed quote");
			i++;
			tokens.push_back(Token(value, true));
			continue;
		}
		if (std::isspace(static_cast<unsigned char>(data[i])))
		{
			if (!current.empty())
			{
				tokens.push_back(current);
				current.clear();
			}
			continue;
		}
		if (data[i] == '{' || data[i] == '}' || data[i] == ';')
		{
			if (!current.empty())
			{
				tokens.push_back(current);
				current.clear();
			}
			tokens.push_back(std::string(1, data[i]));
      continue;
		}
		current += data[i];
	}
	if (!current.empty())
	{
		tokens.push_back(current);
		current.clear();
	}
	return tokens;
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

bool ServerConfig::getHasMaxSize() const
{
	return _hasMaxSize;
}

const std::map<int, std::string> &ServerConfig::getErrorPage() const
{
	return _errorPage;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const
{
	return _locations;
}

std::vector<LocationConfig>& ServerConfig::getLocations()
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

void	ServerConfig::setHasMaxSize(bool v)
{
	_hasMaxSize = v;
}

void	ServerConfig::setErrorPage(int i, std::string s)
{
	_errorPage[i] = s;
}

void	ServerConfig::setLocations(LocationConfig locations)
{
	_locations.push_back(locations);
}

std::ostream &operator<<(std::ostream &os, const ServerConfig &srv)
{
	os << "======== SERVER ========\n";

	os << "ports: ";
	for (size_t i = 0; i < srv.getPort().size(); i++)
		os << srv.getPort()[i] << " ";
	os << "\n";

	os << "server names: ";
	for (size_t i = 0; i < srv.getServerName().size(); i++)
		os << srv.getServerName()[i] << " ";
	os << "\n";

	os << "root: " << srv.getRoot() << "\n";

	os << "index: ";
	for (size_t i = 0; i < srv.getIndex().size(); i++)
		os << srv.getIndex()[i] << " ";
	os << "\n";

	os << "client max body size: " << srv.getBodySizeClient() << "\n";
	os << "has max size: " << srv.getHasMaxSize() << "\n";

	os << "error pages:\n";
	for (std::map<int, std::string>::const_iterator it = srv.getErrorPage().begin();
		 it != srv.getErrorPage().end(); ++it)
		os << "  " << it->first << " -> " << it->second << "\n";

	os << "locations:\n";
	std::vector<LocationConfig> locs = srv.getLocations();
	for (size_t i = 0; i < locs.size(); i++)
		os << locs[i];

	os << "========================\n";
	return os;
}

bool operator==(const Token &t, const std::string &s)
{
	return t.value == s;
}

bool operator!=(const Token &t, const std::string &s)
{
	return t.value != s;
}