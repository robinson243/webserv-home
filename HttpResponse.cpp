/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:09:04 by romukena          #+#    #+#             */
/*   Updated: 2026/04/16 14:15:36 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

HttpResponse::HttpResponse() : _code(-1), _version(NULL), _message(NULL) {
}

HttpResponse::~HttpResponse() {
}

int HttpResponse::getCode() const
{
	return _code;
}

std::string HttpResponse::getVersion() const
{
	return _version;
}

std::string HttpResponse::getMessage() const{
	return _message;
}

std::vector<unsigned char> HttpResponse::getBody() const
{
	return _body;
}

std::map<std::string, std::string> HttpResponse:: getHeaders() const
{
	return _headers;
}