/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: romukena <romukena@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 15:50:44 by romukena          #+#    #+#             */
/*   Updated: 2026/04/27 00:04:31 by romukena         ###   ########.fr       */
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