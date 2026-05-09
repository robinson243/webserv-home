/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:32:24 by oamairi           #+#    #+#             */
/*   Updated: 2026/05/09 13:02:01 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Server.hpp"
#include <vector>
#include <fstream>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include "HttpRequest.hpp"
#include "RequestHandler.hpp"

class EventLoop
{
private:
	std::vector<pollfd>				_fds;
	std::vector<Server>				_servers;
	std::map<int, std::string>		_buffers;
	std::map<int, ServerConfig>		_serverToConfig;
	std::map<int, ServerConfig>		_clientFdToConfig;
public:
	EventLoop(const std::vector<ServerConfig> &configs);
	void	run();
	bool	isServerFd(int fd);
	EventLoop();
	~EventLoop();
};