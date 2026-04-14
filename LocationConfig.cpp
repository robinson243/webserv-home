/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 13:44:36 by romukena          #+#    #+#             */
/*   Updated: 2026/04/14 14:48:58 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include <limits>

LocationConfig::LocationConfig()
	: _autoindex(false), _max_body(std::numeric_limits<std::size_t>::max()),
	  _code(-1), _url(""), _root(""), _path(""), _index(""), _upload_path(""),
	  _alias("") {
}

LocationConfig::~LocationConfig() {
}

// ── Getters ───────────────────────────────────────────────────
bool LocationConfig::getAutoindex() const {
	return _autoindex;
}
size_t LocationConfig::getMaxBody() const {
	return _max_body;
}
int LocationConfig::getCode() const {
	return _code;
}
const std::string &LocationConfig::getUrl() const {
	return _url;
}
const std::string &LocationConfig::getRoot() const {
	return _root;
}
const std::string &LocationConfig::getPath() const {
	return _path;
}
const std::string &LocationConfig::getIndex() const {
	return _index;
}
const std::string &LocationConfig::getUploadPath() const {
	return _upload_path;
}
const std::string &LocationConfig::getAlias() const {
	return _alias;
}
const std::set<std::string> &LocationConfig::getAllowMethods() const {
	return _allow_methods;
}
const std::map<std::string, std::string> &
LocationConfig::getCgiExtension() const {
	return _cgi_extension;
}

// ── Setters ───────────────────────────────────────────────────
void LocationConfig::setAutoindex(bool v) {
	_autoindex = v;
}
void LocationConfig::setMaxBody(size_t v) {
	_max_body = v;
}
void LocationConfig::setCode(int v) {
	_code = v;
}
void LocationConfig::setUrl(const std::string &v) {
	_url = v;
}
void LocationConfig::setRoot(const std::string &v) {
	_root = v;
}
void LocationConfig::setPath(const std::string &v) {
	_path = v;
}
void LocationConfig::setIndex(const std::string &v) {
	_index = v;
}
void LocationConfig::setUploadPath(const std::string &v) {
	_upload_path = v;
}
void LocationConfig::setAlias(const std::string &v) {
	_alias = v;
}
void LocationConfig::setAllowMethods(const std::set<std::string> &v) {
	_allow_methods = v;
}
void LocationConfig::setCgiExtension(
	const std::map<std::string, std::string> &v) {
	_cgi_extension = v;
}

// ── Helpers ───────────────────────────────────────────────────
void LocationConfig::addMethod(const std::string &method) {
	_allow_methods.insert(method);
}

void LocationConfig::addCgiExtension(const std::string &ext,
									 const std::string &interpreter) {
	_cgi_extension[ext] = interpreter;
}

bool LocationConfig::isMethodAllowed(const std::string &method) const {
	if (_allow_methods.empty())
		return true;
	return _allow_methods.find(method) != _allow_methods.end();
}

bool LocationConfig::hasCgi(const std::string &ext) const {
	return _cgi_extension.find(ext) != _cgi_extension.end();
}

bool LocationConfig::hasRedirect() const {
	return _code != -1;
}