/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 13:44:36 by romukena          #+#    #+#             */
/*   Updated: 2026/04/21 17:58:11 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include <limits>
#include <array>

LocationConfig::LocationConfig()
	: _autoindex(false), _has_autoindex(false), _max_body(std::numeric_limits<std::size_t>::max()),
	  _has_max_size(false), _return_code(-1), _return_url(""), _has_return(false), _root(""), _path(""), _upload_path(""),
	  _alias("") {
}

LocationConfig::~LocationConfig() {
}

LocationConfig parseLocation(std::vector<Token>::iterator &it, std::vector<Token>::iterator end)
{
	++it;
	if (it == end)
		throw std::runtime_error("Location empty");
	LocationConfig location;
	std::string path = it->value;
	if (path.empty() || path[0] != '/')
		throw std::runtime_error("Location path must start with '/'");
	location.setPath(path);
	++it;
	if (it == end || *it != "{")
		throw std::runtime_error("Location should start with '{'");
	++it;
	while (it != end && *it != "}")
	{
		if (*it == "root")
		{
			if (!location.getRoot().empty())
				throw std::runtime_error("Mutiple definition of root on location");
			location.setRoot(parseSingleValueDirective(it, end, it->value));
		}
		else if (*it == "alias")
		{
    	if (!location.getAlias().empty())
				throw std::runtime_error("Mutiple definition of Alias on location");
			location.setAlias(parseSingleValueDirective(it, end, it->value));
		}
		else if (*it == "autoindex")
    	parseAutoindex(it, end, location);
		else if (*it == "allow_methods")
    	parseAllowMethods(it, end, location);
		else if (*it == "index")
    	parseIndex(it, end, location);
		else if (*it == "upload_path")
		{
			if (!location.getUploadPath().empty())
				throw std::runtime_error("Mutilple definition of upload path");
			location.setUploadPath(parseSingleValueDirective(it, end, it->value));
		}
		else if (*it == "client_max_body_size")
    {
			if (location.gethasmaxsize() == true)
				throw std::runtime_error("Multiple definitions of body_size");
			location.setMaxBody(findSize(it, end));
			location.sethasmaxsize(true);
		}
		else if (*it == "cgi_extension")
    	parseCgiExtension(it, end, location);
		else if (*it == "return")
    	parseReturn(it, end, location);
		else
    	throw std::runtime_error("Unknown directive in location");
	}
	if (it == end)
		throw std::runtime_error("Missing bracket");
	++it;
	return location;
}

void	parseReturn(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, LocationConfig &location)
{
	int code;
	std::string path;

	if (location.hasRedirect())
		throw std::runtime_error("Multiple definitons of return");
	++it;
	if (it == end)
		throw std::runtime_error("Return: missing value");
	code = std::stoi(it->value);
	if (code < 100 || code > 599)
		throw std::runtime_error("Return: invalid value");
	++it;
	if (it == end)
		throw std::runtime_error("Return: missing value");
	if ((*it == "}" || *it == "{" || *it == ";") && !it->in_quotes)
		throw std::runtime_error("Return: invalid value");
	path = it->value;
	++it;
	if (it == end || *it != ";" || it->in_quotes)
		throw std::runtime_error("Return: missing ';'");
	location.setCode(code);
	location.setUrl(path);
	++it; // skip ;
}

void	parseCgiExtension(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, LocationConfig &location)
{
	std::string extension;
	std::string path;

	++it;
	if (it == end || ((*it == "{" || *it == "}" || *it == ";") && !it->in_quotes))
		throw std::runtime_error("cgi extension: missing value");
	extension = it->value;
	++it;
	if (it == end)
		throw std::runtime_error("cgi extension: missing value");
	if ((*it == "}" || *it == "{" || *it == ";") && !it->in_quotes)
		throw std::runtime_error("cgi extension: invalid value");
	path = it->value;
	++it;
	if (it == end || *it != ";" || it->in_quotes)
		throw std::runtime_error("cgi extension: missing ';'");
	++it; // skip ;
	location.addCgiExtension(extension, path);
}

std::string parseSingleValueDirective(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, const std::string &name)
{
    ++it;
    if (it == end)
        throw std::runtime_error(name + ": missing value");
    if ((*it == "{" || *it == "}" || *it == ";") && !it->in_quotes)
        throw std::runtime_error(name + ": invalid value");
    std::string value = it->value;
    ++it;
    if (it == end || *it != ";" || it->in_quotes)
        throw std::runtime_error(name + ": expected ';'");
		++it; // skip ;
    return value;
}

void parseIndex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, LocationConfig &location)
{
	std::vector<std::string> index;
	std::vector<std::string> tmp = location.getIndex();

	if (!tmp.empty())
		throw std::runtime_error("Multiple definitions of Index");
	++it;
	if (it == end)
		throw std::runtime_error("Index: missing value");
	while (it != end && (*it != ";" || it->in_quotes))
	{
		if (*it == "}" || *it == "{")
			throw std::runtime_error("Index: brace in index forbidden");
		index.push_back(it->value);
		++it;
	}
	if (it == end)
		throw std::runtime_error("Index: unterminated index");
	if (index.empty())
		throw std::runtime_error("Index: no index provided");
	++it; // Skip ;
	location.setIndex(index);
}

void	parseAllowMethods(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, LocationConfig &location)
{
	std::set<std::string> allowmethod;
	const std::set<std::string> tmp = location.getAllowMethods();

	if (!tmp.empty())
		throw std::runtime_error("Multiple definition of allow_methods");
	++it;
	if (it == end)
		throw std::runtime_error("allow_methods: missing value");
	while (it != end && (*it != ";" && !it->in_quotes))
	{
		if (*it == "}" || *it == "{")
			throw std::runtime_error("allow_methods: brace in name forbidden");
		if (*it != "GET" && *it != "POST" && *it != "DELETE")
			throw std::runtime_error("allow_methods: invalid value " + it->value);
		if (!location.isMethodAllowed(it->value))
			throw std::runtime_error("allow_methods: Multiple same method");
		allowmethod.insert(it->value);
		++it;
	}
	if (it == end)
		throw std::runtime_error("allow_methods: unterminated directive");
	if (allowmethod.empty())
		throw std::runtime_error("allow_methods: no names provided");
	++it; // Skip ;
	location.setAllowMethods(allowmethod);
}

void parseAutoindex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, LocationConfig &location)
{
	if (location.gethasAutoindex())
		throw std::runtime_error("Multiple definition of Auto index");
	++it;
	if (it == end)
		throw std::runtime_error("Autoindex: missing autoindex");
	if (*it == "on")
		location.setAutoindex(true);
	else if (*it == "off")
		location.setAutoindex(false);
	else
		throw std::runtime_error("Autoindex: invalid value");
	++it;
	if (it == end || *it != ";" || it->in_quotes)
		throw std::runtime_error("Autoindex: missing ';");
	++it; // skip ;
	location.sethasAutoindex(true);
}

void parseAlias(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, LocationConfig &location)
{
	std::string alias = location.getAlias();
	if (!alias.empty())
		throw std::runtime_error("Mutiple definition of alias on location");
	++it;
	if (it == end)
		throw std::runtime_error("Alias: missing autoindex");
	if (*it == "}" || *it == "{" || *it == ";")
		throw std::runtime_error("Alias: invalid value");
	std::string s = *it;
	++it;
	if (it == end || *it != ";")
		throw std::runtime_error("Alias: expected ';'");
	++it;
	location.setAlias(s);
}

void parseRoot(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, LocationConfig &location)
{
	std::string root = location.getRoot();
	if (!root.empty())
		throw std::runtime_error("Mutiple definition of root on location");
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
	location.setRoot(s);
}

// ── Getters ───────────────────────────────────────────────────
bool LocationConfig::getAutoindex() const {
	return _autoindex;
}

bool LocationConfig::gethasAutoindex() const {
	return _has_autoindex;
}
size_t LocationConfig::getMaxBody() const {
	return _max_body;
}

bool LocationConfig::gethasmaxsize() const
{
	return _has_max_size;
}

int LocationConfig::getCode() const {
	return _return_code;
}
const std::string &LocationConfig::getUrl() const {
	return _return_url;
}
const std::string &LocationConfig::getRoot() const {
	return _root;
}
const std::string &LocationConfig::getPath() const {
	return _path;
}
const std::vector<std::string> &LocationConfig::getIndex() const {
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

void LocationConfig::sethasAutoindex(bool v) {
	_has_autoindex = v;
}

void LocationConfig::setMaxBody(size_t v) {
	_max_body = v;
}

void LocationConfig::sethasmaxsize(bool v)
{
	_has_max_size = v;	
}

void LocationConfig::setCode(int v) {
	_return_code = v;
}
void LocationConfig::setUrl(const std::string &v) {
	_return_url = v;
}
void LocationConfig::setRoot(const std::string &v) {
	_root = v;
}
void LocationConfig::setPath(const std::string &v) {
	_path = v;
}
void LocationConfig::setIndex(const std::vector<std::string> &v) {
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

void LocationConfig::addCgiExtension(const std::string &ext, const std::string &interpreter)
{
	if (this->hasCgi(ext))
		throw std::runtime_error("Duplicate CGI extension: " + ext);
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
	return _has_return;
}

std::ostream &operator<<(std::ostream &os, const LocationConfig &loc)
{
	os << "----- LOCATION -----\n";

	if (loc.gethasAutoindex())
		os << "autoindex: " << loc.getAutoindex() << "\n";

	if (loc.getMaxBody() != 0)
		os << "max_body: " << loc.getMaxBody() << "\n";

	if (loc.hasRedirect())
	{
		os << "return code: " << loc.getCode() << "\n";
		os << "return url: " << loc.getUrl() << "\n";
	}

	// if (!loc.getRoot().empty())
		os << "root: " << loc.getRoot() << "\n";

	if (!loc.getPath().empty())
		os << "path: " << loc.getPath() << "\n";

	if (!loc.getAlias().empty())
		os << "alias: " << loc.getAlias() << "\n";

	if (!loc.getUploadPath().empty())
		os << "upload_path: " << loc.getUploadPath() << "\n";

	if (!loc.getIndex().empty())
	{
		os << "index: ";
		for (size_t i = 0; i < loc.getIndex().size(); i++)
			os << loc.getIndex()[i] << " ";
		os << "\n";
	}

	if (!loc.getAllowMethods().empty())
	{
		os << "allow_methods: ";
		for (std::set<std::string>::const_iterator it = loc.getAllowMethods().begin();
			 it != loc.getAllowMethods().end(); ++it)
			os << *it << " ";
		os << "\n";
	}

	if (!loc.getCgiExtension().empty())
	{
		os << "cgi_extension:\n";
		for (std::map<std::string, std::string>::const_iterator it = loc.getCgiExtension().begin();
			 it != loc.getCgiExtension().end(); ++it)
			os << "  " << it->first << " -> " << it->second << "\n";
	}

	os << "--------------------\n";
	return os;
}