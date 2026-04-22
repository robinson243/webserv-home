/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:16:54 by romukena          #+#    #+#             */
/*   Updated: 2026/04/22 13:47:33 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _isValid(false) {
}

HttpRequest::~HttpRequest() {
}

std::vector<unsigned char> HttpRequest::getBody() const {
	return _body;
}

bool HttpRequest::getValid() const {
	return _isValid;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders() const {
	return _headers;
}

const std::map<std::string, std::string>& HttpRequest::getRequest() const {
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
	while (s >> token) {
		if (i == 0)
			this->addRequest("method", token);
		else if (i == 1)
			this->addRequest("uri", token);
		else if (i == 2)
			this->addRequest("version", token);
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
	this->addHeaders(sub, value);
}

void HttpRequest::addAllHeaders(std::stringstream &str) {
	std::string token;
	while (std::getline(str, token)) {
		if (token == "\r")
			break;
		this->substractAndAdd(token);
	}
}