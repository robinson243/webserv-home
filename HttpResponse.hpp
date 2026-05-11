/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:48:13 by romukena          #+#    #+#             */
/*   Updated: 2026/05/06 01:43:42 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <map>
#include <vector>

/*
HTTP/1.1 200 OK\r\n          ← status line
Content-Type: text/html\r\n  ← headers
Content-Length: 42\r\n       |
\r\n
<html>...</html>             ← body
*/

class HttpResponse
{
private:
	int _code;
	std::string _version;
	std::string _message;
	std::vector<unsigned char> _body;
	std::map<std::string, std::string> _headers;

public:
	HttpResponse();
	~HttpResponse();
	int getCode() const;
	std::string getVersion() const;
	std::string getMessage() const;
	std::vector<unsigned char> getBody() const;
	std::map<std::string, std::string> getHeaders() const;

	void addCode(int code);
	void addVersion(std::string &e);
	void addMessage(std::string &e);
	void addBodyResponse(std::string &e);
	void addHeadersResponse(const std::string &key, const std::string &e);
	void setBody(const std::vector<unsigned char> &body);
	std::string serialize();
};
