/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:01:30 by romukena          #+#    #+#             */
/*   Updated: 2026/04/22 15:49:49 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <climits>
#include <iostream>
#include <map>
#include <set>
#include <vector>

class LocationConfig;

struct Token
{
	std::string value;
	bool in_quotes;

	Token() : value(""), in_quotes(false) {}

	Token(const std::string &v) : value(v), in_quotes(false) {}

	Token(const std::string &v, bool q) : value(v), in_quotes(q) {}
};

class ServerConfig {
private:
	std::vector<std::string> _data;
	std::vector<unsigned int> _port;
	std::vector<std::string> _serverName;
	std::string _root;
	std::vector<std::string> _index;
	size_t _clientMaxBodySize;
	bool	_hasMaxSize;
	std::map<int, std::string> _errorPage;
	std::vector<LocationConfig> _locations;

  public:
	ServerConfig();
	ServerConfig &operator=(const ServerConfig &other);
	ServerConfig(const ServerConfig &other);
	~ServerConfig();

	void	parsConfig(std::vector<std::string> &data);
	std::vector<unsigned int> getPort() const;
	std::vector<std::string> getServerName() const;
	std::string getRoot() const;
	std::vector<std::string> getIndex() const;
	size_t getBodySizeClient() const;
	bool	getHasMaxSize() const;
	const std::map<int, std::string> &getErrorPage() const;
	std::vector<LocationConfig>& getLocations();
	const std::vector<LocationConfig>& getLocations() const;

	void	setPort(unsigned int port);
	void	setServerName(std::vector<std::string> Servername);
	void	setRoot(std::string root);
	void	setIndex(std::vector<std::string> index);
	void	setSizeClient(size_t size);
	void	setHasMaxSize(bool v);
	void	setErrorPage(int i, std::string s);
	void	setLocations(LocationConfig locations);
};

std::ostream &operator<<(std::ostream &os, const ServerConfig &srv);

std::vector<Token> tokenize(const std::string &data);
std::string	LoadConfigFile(const std::string &file);
std::vector<ServerConfig> pars(const std::string &file);
ServerConfig parseServer(std::vector<Token>::iterator &it, std::vector<Token>::iterator end);
void parseDirective(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, ServerConfig &server);
unsigned int findPort(std::vector<Token>::iterator &it, std::vector<Token>::iterator end);
std::vector<std::string> findServerName(std::vector<Token>::iterator &it, std::vector<Token>::iterator end);
std::string findRoot(std::vector<Token>::iterator &it, std::vector<Token>::iterator end);
std::vector<std::string> findIndex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end);
void	parseErrorPage(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, ServerConfig &server);
size_t findSize(std::vector<Token>::iterator &it, std::vector<Token>::iterator end);
void validateServer(ServerConfig &server);
std::map<int, std::vector<ServerConfig*>> groupServersByPort(const std::vector<ServerConfig> &servers);

bool operator==(const Token &t, const std::string &s);
bool operator!=(const Token &t, const std::string &s);

#endif