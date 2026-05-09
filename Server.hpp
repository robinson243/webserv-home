/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 15:33:47 by oamairi           #+#    #+#             */
/*   Updated: 2026/05/09 14:53:28 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <netinet/in.h>
#include "ServerConfig.hpp"

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
