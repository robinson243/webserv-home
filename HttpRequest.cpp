/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:16:54 by romukena          #+#    #+#             */
/*   Updated: 2026/04/16 18:04:04 by romukena         ###   ########.fr       */
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

std::map<std::string, std::string> HttpRequest::getHeaders() const {
	return _headers;
}

std::map<std::string, std::string> HttpRequest::getRequest() const {
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

void HttpRequest::addHeaders(std::string &key, std::string &element) {
	_headers.insert(std::pair<std::string, std::string>(key, element));
}

void HttpRequest::addRequest(std::string &key, std::string &element) {
	_requestLine.insert(std::pair<std::string, std::string>(key, element));
}
