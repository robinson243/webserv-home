/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 12:17:03 by romukena          #+#    #+#             */
/*   Updated: 2026/04/14 12:53:51 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
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

class LocationConfig {
  private:
	bool autoindex;
	size_t max_body;
	std::string root;
	std::string path;
	std::string index;
	std::string upload_path;
	std::string alias;
	int code;
	std::string url;
	std::set<std::string> allow_methods;
	std::map<std::string, std::string> cgi_extension;

  public:
};
