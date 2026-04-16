/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:16:54 by romukena          #+#    #+#             */
/*   Updated: 2026/04/16 14:08:12 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _body(NULL), _isValid(false) {
}

HttpRequest::~HttpRequest() {
}

std::vector<unsigned char> HttpRequest::getBody() const {
	return _body;
}

bool HttpRequest::getValid() const
{
	return _isValid;
}

std::map<std::string, std::string> HttpRequest::getHeaders() const
{
	return _headers;
}

std::map<std::string, std::string> HttpRequest::getRequest() const
{
	return _requestLine;
}