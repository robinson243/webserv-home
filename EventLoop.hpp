/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:32:24 by oamairi           #+#    #+#             */
/*   Updated: 2026/06/25 17:41:43 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <vector>
#include <poll.h>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include "Server.hpp"
#include <sys/socket.h>
#include "HttpRequest.hpp"
#include "RequestHandler.hpp"

class EventLoop
{
private:
	bool										_run;
	std::vector<pollfd>							_fds;
	std::vector<Server>							_servers;
	std::map<int, std::string>					_buffers;
	std::set<int>								_bindedPorts;
	std::map<int, std::vector<ServerConfig> >	_serverToConfig;
	std::map<int, std::vector<ServerConfig> >	_clientFdToConfig;

	static std::string toLowerStr(std::string s);
	static std::string trim(const std::string &s);
public:
	EventLoop(const std::vector<ServerConfig> &configs);
	void	run();
	void	stop();
	bool	isServerFd(int fd);
	~EventLoop();
};