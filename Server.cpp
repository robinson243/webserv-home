/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 15:43:54 by oamairi           #+#    #+#             */
/*   Updated: 2026/05/02 19:15:44 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port) : _port(port), _fd(-1)
{
	bzero(&this->_addr, sizeof(this->_addr));
}

int	Server::getFd()
{
	return this->_fd;
}

int	Server::getPort()
{
	return this->_port;
}

void	Server::setup()
{
	this->_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_fd == -1)
		(perror("socket error"), exit(1));
	// Permet de rendre le socket non bloquant
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		(perror("setsockopt error"), exit(1));
	_addr.sin_port = htons(_port);
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(_fd, (struct sockaddr*) &_addr, sizeof(_addr)) == -1)
		(perror("bind error"), exit(1));
	if (listen(_fd, SOMAXCONN) == -1)
		(perror("listen error"), exit(1));
	int flags = fcntl(_fd, F_GETFL, 0);
	if (flags == -1)
		(perror("fcntl(F_GETFL) error"), exit(1));
	if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		(perror("fcntl(F_SETFL) error"), exit(1));
}

Server::~Server(){};
