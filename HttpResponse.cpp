/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:09:04 by romukena          #+#    #+#             */
/*   Updated: 2026/05/06 01:43:21 by romukena         ###   ########.fr       */
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

static std::string codeReturn(int code) {
	std::string e;
	if (code == 200)
		e = "OK";
	else if (code == 201)
		e = "Created";
	else if (code == 204)
		e = "No Content";
	else if (code == 301)
		e = "Moved Permanently";
	else if (code == 400)
		e = "Bad Request";
	else if (code == 403)
		e = "Forbidden";
	else if (code == 404)
		e = "Not Found";
	else if (code == 405)
		e = "Method Not Allowed";
	else if (code == 413)
		e = "Payload Too Large";
	else if (code == 500)
		e = "Internal Server Error";
	else if (code == 501)
		e = "Not Implemented";
	else if (code == 502)
		e = "Bad Gateway";
	else if (code == 504)
		e = "Gateway Timeout";
	else
		e = "Unknown";
	return e;
}

void HttpResponse::addCode(int code) {
	_code = code;
	_version = "HTTP/1.1";
	_message = codeReturn(code);
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
	_headers[key] = e;
}

void HttpResponse::setBody(const std::vector<unsigned char> &body) {
	_body = body;
}

std::string HttpResponse::serialize() {
    std::string final;
    std::string body(_body.begin(), _body.end());

    std::ostringstream lenOss;
    lenOss << body.size();
    _headers["Content-Length"] = lenOss.str();

    std::ostringstream oss;
    oss << _code;
    final = _version + " " + oss.str() + " " + _message + "\r\n";
    for (std::map<std::string, std::string>::iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        final += it->first + ": " + it->second + "\r\n";
    }
    final += "\r\n" + body;
    return final;
}