/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:32:24 by oamairi           #+#    #+#             */
/*   Updated: 2026/04/30 19:37:05 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Server.hpp"
#include <vector>
#include <poll.h>

class EventLoop
{
private:
	std::vector<pollfd>	_fds;
	std::vector<Server>	_servers;
public:
	EventLoop(const std::vector<ServerConfig> &configs);
	void	run();
	bool	isServerFd(int fd);
	~EventLoop();
};