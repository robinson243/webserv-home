/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:01:30 by romukena          #+#    #+#             */
/*   Updated: 2026/04/20 21:06:09 by ydembele         ###   ########.fr       */
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
	ServerConfig(const std::string data);
	ServerConfig &operator=(ServerConfig &other);
	~ServerConfig();

	void	parsConfig(std::vector<std::string> &data);
	std::vector<unsigned int> getPort() const;
	std::vector<std::string> getServerName() const;
	std::string getRoot() const;
	std::vector<std::string> getIndex() const;
	size_t getBodySizeClient() const;
	bool	getHasMaxSize() const;
	const std::map<int, std::string> &getErrorPage() const;
	std::vector<LocationConfig> getLocations() const;

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

std::vector<std::string> tokenize(const std::string &data);
std::string	LoadConfigFile(const std::string &file);
std::vector<ServerConfig> pars(const std::string &file);
ServerConfig parseServer(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
void parseDirective(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, ServerConfig &server);
unsigned int findPort(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
std::vector<std::string> findServerName(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
std::string findRoot(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
std::vector<std::string> findIndex(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
void	parseErrorPage(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, ServerConfig &server);
size_t findSize(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);

#endif
