/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 15:16:52 by romukena          #+#    #+#             */
/*   Updated: 2026/04/16 17:33:02 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>
#include <vector>

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

class HttpRequest {
  private:
	std::vector<unsigned char> _body;
	bool _isValid;
	std::map<std::string, std::string> _headers;
	std::map<std::string, std::string> _requestLine;

  public:
	HttpRequest();
	~HttpRequest();
	std::vector<unsigned char> getBody() const;
	bool getValid() const;
	std::map<std::string, std::string> getHeaders() const;
	std::map<std::string, std::string> getRequest() const;

	void addBody(std::string &element);
	void makeTrue();
	void addHeaders(std::string &key, std::string &element);
	void addRequest(std::string &key, std::string &element);
};
