/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:32:24 by oamairi           #+#    #+#             */
/*   Updated: 2026/05/02 14:24:55 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Server.hpp"
#include <vector>
#include <fstream>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

class EventLoop
{
private:
	std::vector<pollfd>			_fds;
	std::vector<Server>			_servers;
	std::map<int, std::string>	_buffers;
public:
	EventLoop(const std::vector<ServerConfig> &configs);
	void	run();
	bool	isServerFd(int fd);
	~EventLoop();
};