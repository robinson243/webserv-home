/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:16:52 by romukena          #+#    #+#             */
/*   Updated: 2026/04/14 15:47:11 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <vector>
#include <iostream>

/*
┌─────────────────────────────────────────────┐
│             REQUEST LINE                    │
│  POST  /upload/fichier.txt  HTTP/1.1        │
│   │           │                │            │
│ method       uri           version          │
└─────────────────────────────────────────────┘
                    \r\n
┌─────────────────────────────────────────────┐
│               HEADERS                       │
│  Host: localhost:8080                       │
│  Content-Type: text/plain                   │
│  Content-Length: 29                         │
│  Connection: keep-alive                     │
└─────────────────────────────────────────────┘
                \r\n\r\n  ← ligne vide = séparateur OBLIGATOIRE
┌─────────────────────────────────────────────┐
│                 BODY                        │
│  Bonjour, voici mon fichier !               │
│  (exactement 29 octets)                     │
└─────────────────────────────────────────────┘
*/

class HttpRequest
{
private:
	std::vector<unsigned char>			_body;
	bool								_isValid;
	std::map<std::string, std::string>	_headers;
	std::map<std::string, std::string>	_requestLine;
public:
	HttpRequest(/* args */);
	~HttpRequest();
};

