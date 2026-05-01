/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 13:40:39 by ydembele          #+#    #+#             */
/*   Updated: 2026/05/01 14:50:23 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

#include <cstdlib>
#include <limits>
#include <fstream>
#include <string>
#include <fstream>
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
        _listenHost = other._listenHost;
        _port = other._port;
        _serverName = other._serverName;
        _root = other._root;
        _index = other._index;
        _clientMaxBodySize = other._clientMaxBodySize;
        _errorPage = other._errorPage;
        _locations = other._locations;
        _hasMaxSize = other._hasMaxSize;
        _data = other._data;
    }
    return *this;
}

ServerConfig::ServerConfig(const ServerConfig &other)
{
    _listenHost = other._listenHost;
    _port = other._port;
    _serverName = other._serverName;
    _root = other._root;
    _index = other._index;
    _clientMaxBodySize = other._clientMaxBodySize;
    _errorPage = other._errorPage;
    _locations = other._locations;
    _hasMaxSize = other._hasMaxSize;
    _data = other._data;
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
		{
        	throw std::runtime_error("Expected 'server' " + it->value);
		}
		ServerConfig server = parseServer(it, tokens.end());
		validateServer(server);
    	servers.push_back(server);
	}
	if (servers.empty())
		throw std::runtime_error("Servers empty");
	return servers;
}

std::map<int, std::vector<ServerConfig*> > groupServersByPort(const std::vector<ServerConfig> &servers)
{
	std::map<int, std::vector<ServerConfig*> > serversByPort;
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
	std::string directives[7] = {"listen", "server_name", "root", "index", "error_page", "client_max_body_size", "location"};
	int i = 0;
	while (i < 7 && it->value != directives[i])
		i++;
	if (i == 7)
		throw std::runtime_error("Unknown directive: " + it->value);
	std::cout << it->value << std::endl;
	switch (i)
	{
		case 0:
			findPort(it, end, server);
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
	char *d;
	long value = std::strtol(it->value.c_str(), &d, 10);

	if (*d != '\0')
    	throw std::runtime_error("client_max_body_size: invalid number");
	if (value < 0)
    	throw std::runtime_error("client_max_body_size: negative value");
	++it;
	if (it == end || *it != ";" || it->in_quotes)
		throw std::runtime_error("client_max_body_size: Body size: missing ';");
	++it;
	return static_cast<size_t>(value);
}

void	parseErrorPage(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, ServerConfig &server)
{
	std::string path;

	++it;
	if (it == end)
		throw std::runtime_error("Error page: missing value");
	char *d;
	long code;
	
	code = std::strtol(it->value.c_str(), &d, 10);
	
	if (*d != '\0')
		throw std::runtime_error("Error page: invalid number");
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
	server.setErrorPage(static_cast<int>(code), path);
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

bool isValidHost(const std::string &host)
{
    if (host.empty())
        return false;
    if (host == "localhost")
        return true;
    if (host == "0.0.0.0")
        return true;

    int parts = 0;
    size_t i = 0;
    while (i < host.size())
    {
        if (parts == 4)
            return false;
        size_t start = i;
        while (i < host.size() && host[i] != '.')
        {
            if (!std::isdigit(host[i]))
                return false;
            i++;
        }

        if (start == i)
            return false;
        std::string part = host.substr(start, i - start);

        if (part.size() > 1 && part[0] == '0')
            return false;
        int value = std::atoi(part.c_str());
        if (value < 0 || value > 255)
            return false;
        parts++;
        if (i < host.size() && host[i] == '.')
            i++;
    }

    return (parts == 4);
}

void findPort(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, ServerConfig &server)
{
    ++it;
    if (it == end)
        throw std::runtime_error("listen: missing value");
    if ((*it == "}" && !it->in_quotes) || (*it == "{" && !it->in_quotes))
        throw std::runtime_error("listen: invalid token");

    std::string value = it->value;
    std::string host = "0.0.0.0";
    std::string portStr;
    size_t colon = value.find(':');
    if (colon != std::string::npos)
    {
        host = value.substr(0, colon);
		if (!isValidHost(host))
			throw std::runtime_error("listen: invalid host");
        portStr = value.substr(colon + 1);

        if (host.empty() || portStr.empty())
            throw std::runtime_error("listen: invalid host:port format");
        server.setListenHost(host);
    }
    else
    {
        portStr = value;
        server.setListenHost("0.0.0.0");
    }
    long port = std::strtol(portStr.c_str(), NULL, 10);
    if (port < 1 || port > 65535)
        throw std::runtime_error("listen: invalid port range [1-65535]");
    ++it;
    if (it == end || *it != ";" || it->in_quotes)
        throw std::runtime_error("listen: expected ';'");
    ++it; // skip ;
    server.setPort((unsigned int)port);
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
	std::ifstream f(file.c_str());

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

std::vector<std::string> ServerConfig::getListenHosts() const
{
	return _listenHost;
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

void	ServerConfig::setListenHost(std::string s)
{
	
	_listenHost.push_back(s);
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


ServerConfig *selectServer(int port, std::string host, std::vector<ServerConfig> &servers)
{
    std::map<int, std::vector<ServerConfig*> > serversByPort = groupServersByPort(servers);
    std::map<int, std::vector<ServerConfig*> >::iterator it = serversByPort.find(port);
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

// LocationConfig selectLocation(std::string uri, ServerConfig &servers)
// {
//     std::vector<LocationConfig> locations = servers.getLocations();
//     LocationConfig *l = NULL;
//     if (locations.empty())
//     {
//         l->setPath("/");
//         l->setRoot(servers.getRoot());
//         l->setIndex(servers.getIndex());
//         l->setMaxBody(servers.getBodySizeClient());
//         return *l;
//     }
//     for (size_t i = 0; i < locations.size(); i++)
//     {
//         std::string path = locations[i].getPath();
//         if (uri.compare(0, path.size(), path) == 0)
//         {
//             if (!l || path.size() > l->getPath().size())
//                 l = &locations[i];
//         }
//     }
//     if (!l)
//         return locations[0];
//     return *l;
// }

// std::vector<ListenSocket> buildListenSockets(std::vector<ServerConfig> &servers)
// {
//     std::vector<ListenSocket> result;

//     for (size_t i = 0; i < servers.size(); i++)
//     {
//         ServerConfig &srv = servers[i];
//         std::vector<unsigned int> ports = srv.getPort();
//         std::vector<std::string> hosts = srv.getListenHosts();
//         for (size_t p = 0; p < ports.size(); p++)
//         {
//             std::string host = "0.0.0.0";
//             if (p < hosts.size())
//                 host = hosts[p];
//             else if (!hosts.empty())
//                 host = hosts[0];
//             unsigned int port = ports[p];
//             bool found = false;

//             for (size_t j = 0; j < result.size(); j++)
//             {
//                 if (result[j].host == host && result[j].port == port)
//                 {
//                     result[j].servers.push_back(&srv);
//                     found = true;
//                     break;
//                 }
//             }
//             if (found)
//                 continue;

//             ListenSocket ls;

//             ls.host = host;
//             ls.port = port;
//             ls.servers.push_back(&srv);
//             ls.fd = socket(AF_INET, SOCK_STREAM, 0);
//             if (ls.fd < 0)
//                 throw std::runtime_error("socket failed");
//             int opt = 1;
//             setsockopt(ls.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
//             ls.addr.sin_family = AF_INET;
//             ls.addr.sin_port = htons(port);
//             ls.addr.sin_addr.s_addr = inet_addr(host.c_str());
//             if (bind(ls.fd, (sockaddr*)&ls.addr, sizeof(ls.addr)) < 0)
//                 throw std::runtime_error("bind failed");
//             if (listen(ls.fd, 128) < 0)
//                 throw std::runtime_error("listen failed");
//             result.push_back(ls);
//         }
//     }
//     return result;
// }


