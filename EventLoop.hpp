/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:32:24 by oamairi           #+#    #+#             */
/*   Updated: 2026/05/09 16:10:52 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <vector>
#include <poll.h>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include "Server.hpp"
#include <sys/socket.h>
#include "HttpRequest.hpp"
#include "RequestHandler.hpp"

class EventLoop
{
private:
	std::vector<pollfd>							_fds;
	std::vector<Server>							_servers;
	std::map<int, std::string>					_buffers;
	std::set<int>								_bindedPorts;
	std::map<int, std::vector<ServerConfig> >	_serverToConfig;
	std::map<int, std::vector<ServerConfig> >	_clientFdToConfig;
public:
	EventLoop(const std::vector<ServerConfig> &configs);
	void	run();
	bool	isServerFd(int fd);
	~EventLoop();
};