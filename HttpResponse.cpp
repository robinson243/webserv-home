/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ydembele <ydembele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:09:04 by romukena          #+#    #+#             */
/*   Updated: 2026/05/01 15:50:44 by ydembele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse() : _code(-1) {
}

HttpResponse::~HttpResponse() {
}

int HttpResponse::getCode() const {
	return _code;
}

std::string HttpResponse::getVersion() const {
	return _version;
}

std::string HttpResponse::getMessage() const {
	return _message;
}

std::vector<unsigned char> HttpResponse::getBody() const {
	return _body;
}

std::map<std::string, std::string> HttpResponse::getHeaders() const {
	return _headers;
}

void HttpResponse::addCode(int code) {
	_code = code;
}
void HttpResponse::addVersion(std::string &e) {
	_version = e;
}

void HttpResponse::addMessage(std::string &e) {
	_message = e;
}

void HttpResponse::addBodyResponse(std::string &e) {
	_body.insert(_body.end(), e.begin(), e.end());
}

void HttpResponse::addHeadersResponse(const std::string &key,
									  const std::string &e) {
	_headers.insert(std::pair<std::string, std::string>(key, e));
}

std::string HttpResponse::serialize() {
	std::string final;
	std::ostringstream oss;
	oss << _code;
	final = _version + " " + oss.str() + " " + _message + "\r\n";
	std::map<std::string, std::string>::iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it) {
		std::string str;
		str = it->first + ": " + it->second + "\r\n";
		final += str;
	}
	std::string body(_body.begin(), _body.end());
	final += "\r\n" + body;
	return final;
}
