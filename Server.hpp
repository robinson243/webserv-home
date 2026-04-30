/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 15:33:47 by oamairi           #+#    #+#             */
/*   Updated: 2026/04/30 17:50:08 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ServerConfig.hpp"
#include <cstring>
#include <netinet/in.h>
#include <fcntl.h>

class Server
{
private:
	int					_fd;
	int					_port;
	struct sockaddr_in	_addr;
public:
	Server(int port);
	void	setup();
	int		getFd();
	int		getPort();
	~Server();
};
