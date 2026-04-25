/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:09:04 by romukena          #+#    #+#             */
/*   Updated: 2026/04/25 13:06:17 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

HttpResponse::HttpResponse() : _code(-1), _version(NULL), _message(NULL)
{
}

HttpResponse::~HttpResponse()
{
}

int HttpResponse::getCode() const
{
	return _code;
}

std::string HttpResponse::getVersion() const
{
	return _version;
}

std::string HttpResponse::getMessage() const
{
	return _message;
}

std::vector<unsigned char> HttpResponse::getBody() const
{
	return _body;
}

std::map<std::string, std::string> HttpResponse::getHeaders() const
{
	return _headers;
}

void HttpResponse::addCode(int code)
{
	_code = code;
}
void HttpResponse::addVersion(std::string &e)
{
	_version = e;
}

void HttpResponse::addMessage(std::string &e)
{
	_message = e;
}

void HttpResponse::addBodyResponse(std::string &e)
{
	_body.insert(_body.end(), e.begin(), e.end());
}

void HttpResponse::addHeadersResponse(const std::string &key, const std::string &e)
{
	_headers.insert(std::pair<std::string, std::string>(key, e));
}