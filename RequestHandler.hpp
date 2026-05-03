/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 15:50:44 by romukena          #+#    #+#             */
/*   Updated: 2026/05/03 21:06:08 by romukena         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

int findLocation(ServerConfig server, HttpRequest req);
std::string concatenatePath(ServerConfig server, HttpRequest req);
bool readFileToString(const std::string &path, std::string &content);
std::string getContentType(const std::string &path);
HttpResponse Get(const HttpRequest &req, const ServerConfig &server);
HttpResponse Delete(const HttpRequest &req, const ServerConfig &server);
HttpResponse Post(const HttpRequest &req, const ServerConfig &server);
HttpResponse handleRequest(const HttpRequest &req, const ServerConfig &server);
bool isCgiRequest(const HttpRequest& req, const LocationConfig& loc);