/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:16:54 by romukena          #+#    #+#             */
/*   Updated: 2026/04/24 14:11:21 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _isValid(false), _code(-1) {
}

HttpRequest::~HttpRequest() {
}

std::vector<unsigned char> HttpRequest::getBody() const {
	return _body;
}

bool HttpRequest::getValid() const {
	return _isValid;
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const {
	return _headers;
}

const std::map<std::string, std::string> &HttpRequest::getRequest() const {
	return _requestLine;
}

void HttpRequest::addBody(std::string &element) {
	if (&element == NULL)
		return;
	_body.insert(_body.end(), element.begin(), element.end());
}

void HttpRequest::makeTrue() {
	_isValid = true;
}

void HttpRequest::addHeaders(const std::string &key, std::string &element) {
	_headers.insert(std::pair<std::string, std::string>(key, element));
}

void HttpRequest::addRequest(const std::string &key, std::string &element) {
	_requestLine.insert(std::pair<std::string, std::string>(key, element));
}

void HttpRequest::print() const {
	std::cout << "=== REQUEST LINE ===" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it =
			 _requestLine.begin();
		 it != _requestLine.end();
		 ++it)
		std::cout << it->first << ": " << it->second << std::endl;

	std::cout << "=== HEADERS ===" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it =
			 _headers.begin();
		 it != _headers.end();
		 ++it)
		std::cout << it->first << ": " << it->second << std::endl;

	std::cout << "=== BODY ===" << std::endl;
	std::cout << std::string(_body.begin(), _body.end()) << std::endl;

	std::cout << "=== STATUS ===" << std::endl;
	std::cout << "code: " << _code << std::endl;
	std::cout << "valid: " << _isValid << std::endl;
}

size_t HttpRequest::requestLength(std::string &e) {
	std::stringstream str(e);
	std::string token;
	size_t i = 0;
	while (str >> token) {
		i++;
	}
	return i;
}

void HttpRequest::addRequestLine(std::stringstream &str) {
	std::string line;
	std::getline(str, line);
	std::stringstream s(line);
	std::string token;
	size_t i = 0;
	if (requestLength(line) != 3) {
		_code = 400;
		return;
	}
	while (s >> token) {
		if (i == 0) {
			if (token != "GET" && token != "POST" && token != "DELETE") {
				_code = 501;
				return;
			}
			addRequest("method", token);
		} else if (i == 1)
			addRequest("uri", token);
		else if (i == 2) {
			if (token != "HTTP/1.1") {
				_code = 505;
				return;
			}
			addRequest("version", token);
		}
		i++;
	}
}

void HttpRequest::substractAndAdd(std::string &line) {
	std::stringstream s(line);
	std::string key;
	std::string value;
	s >> key;
	std::string sub = key.substr(0, key.length() - 1);
	s >> value;
	addHeaders(sub, value);
}

void HttpRequest::addAllHeaders(std::stringstream &str) {
	std::string token;
	while (std::getline(str, token)) {
		if (token == "\r")
			break;
		if (token.find(":") == std::string::npos || token.find(":") == 0
			|| token.empty()) {
			_code = 400;
			return;
		}
		substractAndAdd(token);
	}
}

bool HttpRequest::findHostInHeaders() {
	std::map<std::string, std::string> headers = getHeaders();
	if (headers["Host"].empty())
		return false;
	return true;
}

bool HttpRequest::isNumber(std::string &e) {
	for (size_t i = 0; e[i]; i++) {
		if (!isdigit(e[i])) {
			return false;
		}
	}
	return true;
}

bool HttpRequest::validateBody(std::string &e) {
	std::map<std::string, std::string> headers = getHeaders();
	std::map<std::string, std::string>::iterator it =
		headers.find("Content-Length");
	if (it == headers.end())
		return true;

	std::string contentLength = it->second;

	if (!isNumber(contentLength))
		return false;
	int numContentLength = std::stoi(contentLength);
	if (numContentLength != e.length()) {
		_code = 400;
		return false;
	}
	return true;
}

void HttpRequest::addHttpRequest(std::string &req) {
	std::stringstream str(req);
	addRequestLine(str);
	addAllHeaders(str);
	if (!findHostInHeaders()) {
		_code = 400;
		return;
	}
	std::string line;
	std::getline(str, line);
	if (!validateBody(line)) {
		_code = 400;
		return;
	}
	addBody(line);
	if (_code == -1)
	{
		_code = 200;
		makeTrue();
	}
	
}