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

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include <sys/stat.h>
#include <sys/wait.h>

bool isImplementedMethod(const std::string &m);

std::string buildAllowHeader(const std::set<std::string> &allow);

// HttpResponse makeRedirectResponse(int code, const std::string &url);

HttpResponse makeResponse(int code);

std::set<std::string> defaultAllowedMethodsIfEmpty(std::set<std::string> allow);

int findLocation(const ServerConfig &server, const HttpRequest &req);

std::string concatenatePath(const ServerConfig &server, const HttpRequest &req);

bool readFileToString(const std::string &path, std::string &content);

std::string getContentType(const std::string &path);

std::string concatenateLocationPath(const LocationConfig &loc,
									const HttpRequest &req);
HttpResponse Get(const HttpRequest &req, const ServerConfig &server);

HttpResponse Delete(const HttpRequest &req, const ServerConfig &server);

HttpResponse Post(const HttpRequest &req, const ServerConfig &server);

void fillDefaultErrorBody(HttpResponse &resp);

ServerConfig selectServer(const std::vector<ServerConfig> &servers,
						  const HttpRequest &req);

HttpResponse handleRequest(const HttpRequest &req,
						   const std::vector<ServerConfig> &servers);