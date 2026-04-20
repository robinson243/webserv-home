/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 12:17:03 by romukena          #+#    #+#             */
/*   Updated: 2026/04/20 21:56:25 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <climits>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "ServerConfig.hpp"
/*
	---- ARGS ----

	autoindex;
	index;
	root;
	retour de reponse ex (200, 201...);
	cgi_extension ex (.py, .c);
	allow_methods ex(GET, POST ...);
	upload_path;
	alias;
*/

/*
	autoindex    → false
	max_body     → SIZE_MAX   // "illimité"
	code         → -1         // "pas de redirection"
	root         → ""
	path         → ""
	index        → ""
	upload_path  → ""
	alias        → ""
	allow_methods → {} (vide)
	cgi_extension → {} (vide)
*/

// Faire les getters et les setters;
// Faire le constructeur de defaut;

class LocationConfig {
  private:
	bool _autoindex;
	bool _has_autoindex;
	size_t _max_body;
	bool	 _has_max_size;
	int _return_code;
	std::string _return_url;
	bool	_has_return;
	std::string _root;
	std::string _path;
	std::vector<std::string> _index;
	std::string _upload_path;
	std::string _alias;
	std::set<std::string> _allow_methods;
	std::map<std::string, std::string> _cgi_extension;

  public:
	LocationConfig();
	~LocationConfig();

	// Getters
	bool getAutoindex() const;
	bool gethasAutoindex() const;
	size_t getMaxBody() const;
	bool gethasmaxsize() const;
	int getCode() const;
	const std::string &getUrl() const;
	const std::string &getRoot() const;
	const std::string &getPath() const;
	const std::vector<std::string> &getIndex() const;
	const std::string &getUploadPath() const;
	const std::string &getAlias() const;
	const std::set<std::string> &getAllowMethods() const;
	const std::map<std::string, std::string> &getCgiExtension() const;

	// Setters
	void setAutoindex(bool v);
	void setMaxBody(size_t v);
	void sethasmaxsize(bool v);
	void setCode(int v);
	void setUrl(const std::string &v);
	void setRoot(const std::string &v);
	void setPath(const std::string &v);
	void setIndex(const std::vector<std::string> &v);
	void setUploadPath(const std::string &v);
	void setAlias(const std::string &v);
	void setAllowMethods(const std::set<std::string> &v);
	void setCgiExtension(const std::map<std::string, std::string> &v);
	void sethasAutoindex(bool v);

	// Helpers
	void addMethod(const std::string &method);
	void addCgiExtension(const std::string &ext,
						 const std::string &interpreter);
	bool isMethodAllowed(const std::string &method) const;
	bool hasCgi(const std::string &ext) const;
	bool hasRedirect() const;

};

std::ostream &operator<<(std::ostream &os, const LocationConfig &loc);

void parseRoot(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, LocationConfig &location);
void parseAlias(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, LocationConfig &location);
void parseAutoindex(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, LocationConfig &location);
void	parseAllowMethods(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, LocationConfig &location);
void parseIndex(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, LocationConfig &location);
std::string parseSingleValueDirective(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, const std::string &name);
void	parseCgiExtension(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, LocationConfig &location);
LocationConfig parseLocation(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
void	parseReturn(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, LocationConfig &location);

#endif
