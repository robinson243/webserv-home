/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 12:17:03 by romukena          #+#    #+#             */
/*   Updated: 2026/04/14 14:48:02 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <climits>
#include <iostream>
#include <map>
#include <set>
#include <vector>

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
	size_t _max_body;
	int _code;
	std::string _url;
	std::string _root;
	std::string _path;
	std::string _index;
	std::string _upload_path;
	std::string _alias;
	std::set<std::string> _allow_methods;
	std::map<std::string, std::string> _cgi_extension;

  public:
	LocationConfig();
	~LocationConfig();

	// Getters
	bool getAutoindex() const;
	size_t getMaxBody() const;
	int getCode() const;
	const std::string &getUrl() const;
	const std::string &getRoot() const;
	const std::string &getPath() const;
	const std::string &getIndex() const;
	const std::string &getUploadPath() const;
	const std::string &getAlias() const;
	const std::set<std::string> &getAllowMethods() const;
	const std::map<std::string, std::string> &getCgiExtension() const;

	// Setters
	void setAutoindex(bool v);
	void setMaxBody(size_t v);
	void setCode(int v);
	void setUrl(const std::string &v);
	void setRoot(const std::string &v);
	void setPath(const std::string &v);
	void setIndex(const std::string &v);
	void setUploadPath(const std::string &v);
	void setAlias(const std::string &v);
	void setAllowMethods(const std::set<std::string> &v);
	void setCgiExtension(const std::map<std::string, std::string> &v);

	// Helpers
	void addMethod(const std::string &method);
	void addCgiExtension(const std::string &ext,
						 const std::string &interpreter);
	bool isMethodAllowed(const std::string &method) const;
	bool hasCgi(const std::string &ext) const;
	bool hasRedirect() const;
};
