/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:32:40 by oamairi           #+#    #+#             */
/*   Updated: 2026/05/07 17:16:02 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventLoop.hpp"

EventLoop::EventLoop(const std::vector<ServerConfig> &configs)
{
	for (size_t i = 0; i < configs.size(); i++)
	{
		std::vector<unsigned int> ports = configs[i].getPort();
		for (size_t j = 0; j < ports.size(); j++)
		{
			Server server(ports[j]);
			_servers.push_back(server);
			_servers.back().setup();
			struct pollfd polfd;
			polfd.fd = _servers.back().getFd();
			polfd.events = POLLIN;
			polfd.revents = 0;
			_fds.push_back(polfd);
		}
	}
	_configs = configs;
}

bool	EventLoop::isServerFd(int fd)
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (_servers[i].getFd() == fd)
			return true;
	}
	return false;
}

void	EventLoop::run()
{
	while (true)
	{
		if (poll(_fds.data(), _fds.size(), -1) == -1)
			(perror("poll error"), exit(1));
		
		std::vector<pollfd> temp;

		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents == 0)
				continue;
			if (_fds[i].revents & POLLIN)
			{
				if (isServerFd(_fds[i].fd) == true)
				{
					int clientFd = accept(_fds[i].fd, NULL, NULL);
					if (clientFd == -1)
					{
						perror("accept error");
						continue;
					}
					int flags = fcntl(clientFd, F_GETFL, 0);
					if (flags == -1)
						(perror("fcntl(F_GETFL) error"), exit(1));
					if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK | FD_CLOEXEC) == -1)
						(perror("fcntl(F_SETFL) error"), exit(1));
					struct pollfd polfd;
					polfd.fd = clientFd;
					polfd.events = POLLIN;
					polfd.revents = 0;
					temp.push_back(polfd);
					_buffers[clientFd] = "";
				}
				else
				{
					char buffer[4096];
					int read = recv(_fds[i].fd, buffer, 4096, 0);
					if (read <= 0)
					{
						if (read < 0)
							perror("recv error");
						close(_fds[i].fd);
						_buffers.erase(_fds[i].fd);
						_fds.erase(_fds.begin() + i);
						i--;
					}
					else
					{
						_buffers[_fds[i].fd].append(buffer, read);
						if (_buffers[_fds[i].fd].find("\r\n\r\n") != std::string::npos)
						{
							if (_buffers[_fds[i].fd].find("Content-Length: ") != std::string::npos)
							{
								int ligneContentLength = _buffers[_fds[i].fd].find("Content-Length: ");
								int contentLength = std::stoi(_buffers[_fds[i].fd].substr(ligneContentLength + 16));
								std::string body = _buffers[_fds[i].fd].substr(_buffers[_fds[i].fd].find("\r\n\r\n") + 4);
								if (contentLength > 0 && body.size() < contentLength)
									continue;
							}
							HttpRequest request;
							request.addHttpRequest(_buffers[_fds[i].fd]);
							_buffers[_fds[i].fd].clear();
							
							for (size_t i = 0; i < _servers.size(); i++)
							{
								for (size_t j = 0; j < _configs[i].getPort().size(); j++)
								{
									if (_servers[i].getPort() == _configs[i].getPort()[j])
									{
										
										handleRequest(request, _configs[i]);
									}
								}
								
							}
							
						}
					}
				}
			}
		}
		for (size_t i = 0; i < temp.size(); i++)
		{
			_fds.push_back(temp[i]);
		}
	}
}

EventLoop::~EventLoop() {};
