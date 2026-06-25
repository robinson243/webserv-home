/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oamairi <oamairi@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:32:40 by oamairi           #+#    #+#             */
/*   Updated: 2026/06/25 17:52:19 by oamairi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventLoop.hpp"

EventLoop::EventLoop(const std::vector<ServerConfig> &configs) : _run(true)
{
	for (size_t i = 0; i < configs.size(); i++)
	{
		std::vector<unsigned int> ports = configs[i].getPort();
		for (size_t j = 0; j < ports.size(); j++)
		{
			if (_bindedPorts.count(ports[j]))
			{
				for (size_t k = 0; k < _servers.size(); k++)
				{
					if (_servers[k].getPort() == ports[j])
						_serverToConfig[_servers[k].getFd()].push_back(configs[i]);
				}
				continue;
			}
			_bindedPorts.insert(ports[j]);
			Server server(ports[j]);
			_servers.push_back(server);
			_servers.back().setup();
			struct pollfd polfd;
			polfd.fd = _servers.back().getFd();
			polfd.events = POLLIN;
			polfd.revents = 0;
			_fds.push_back(polfd);
			_serverToConfig[_servers.back().getFd()].push_back(configs[i]);
		}
	}
}

std::string	EventLoop::trim(const std::string &s) {
	size_t start = 0;
	while (start < s.size()
		   && std::isspace(static_cast<unsigned char>(s[start])))
		++start;

	size_t end = s.size();
	while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
		--end;

	return s.substr(start, end - start);
}

std::string	EventLoop::toLowerStr(std::string s) {
	std::transform(
		s.begin(), s.end(), s.begin(), static_cast<int (*)(int)>(std::tolower));
	return s;
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
	while (_run)
	{
		if (poll(_fds.data(), _fds.size(), -1) == -1)
		{
			if (errno == EINTR)
			{
				std::cout << "\nwebserv killed by SIGINT\nGoodbye !\n";
				break;
			}
			perror("poll error");
			break;
		}
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
					{
						perror("fcntl(F_GETFL) error");
						break;
					}
					if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK | FD_CLOEXEC) == -1)
					{
						perror("fcntl(F_SETFL) error");
						break;
					}
					struct pollfd polfd;
					polfd.fd = clientFd;
					polfd.events = POLLIN;
					polfd.revents = 0;
					temp.push_back(polfd);
					_buffers[clientFd] = "";
					_clientFdToConfig[clientFd] = _serverToConfig[_fds[i].fd];
				}
				else
				{
					char buffer[131072];
					int read = recv(_fds[i].fd, buffer, 131072, 0);
					if (read <= 0)
					{
						if (read < 0)
						{
							if (errno == ECONNRESET)
								std::cout << "Connection reset by peer, fd : " << _fds[i].fd << "\n";
							else
								perror("recv error");
						}
						close(_fds[i].fd);
						_buffers.erase(_fds[i].fd);
						_clientFdToConfig.erase(_fds[i].fd);
						_fds.erase(_fds.begin() + i);
						i--;
					}
					else
					{
						_buffers[_fds[i].fd].append(buffer, read);
						if (_buffers[_fds[i].fd].find("\r\n\r\n") != std::string::npos)
						{
							if (toLowerStr(_buffers[_fds[i].fd]).find(("content-length:")) != std::string::npos)
							{
								int ligneContentLength = trim(toLowerStr(_buffers[_fds[i].fd])).find("content-length:");
								int contentLength = std::atoi(trim(toLowerStr(_buffers[_fds[i].fd])).substr(ligneContentLength + 15).c_str());
								std::string body = _buffers[_fds[i].fd].substr(_buffers[_fds[i].fd].find("\r\n\r\n") + 4);
								if (contentLength > 0 && (int) body.size() < contentLength)
									continue;
							}
							else if (toLowerStr(_buffers[_fds[i].fd]).find("transfer-encoding:") != std::string::npos)
							{
								if (_buffers[_fds[i].fd].find("0\r\n\r\n") == std::string::npos)
									continue;
							}
							HttpRequest request;
							request.addHttpRequest(_buffers[_fds[i].fd]);
							_buffers[_fds[i].fd].clear();
							HttpResponse response;
							response = handleRequest(request, _clientFdToConfig[_fds[i].fd]);
							std::string raw = response.serialize();
							long long	total = 0;
							long long	sent = 0;
							while ((size_t) total < raw.size())
							{
								sent = send(_fds[i].fd, raw.c_str() + total, raw.size() - total, 0);
								if (sent < 0)
								{
									if (errno == EAGAIN || errno == EWOULDBLOCK)
										continue;
									perror("send error");
									break;
								}
								total = total + sent;
							}
							std::map<std::string, std::string> headers = response.getHeaders();
							std::map<std::string, std::string>::iterator connIt = headers.find("connection");
							if (connIt != headers.end() && connIt->second == "close")
							{
								close(_fds[i].fd);
								_buffers.erase(_fds[i].fd);
								_clientFdToConfig.erase(_fds[i].fd);
								_fds.erase(_fds.begin() + i);
								i--;
							}
							else
								_buffers[_fds[i].fd].clear();
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

void	EventLoop::stop()
{
	_run = false;
}

EventLoop::~EventLoop() 
{
	for (size_t i = 0; i < _fds.size(); i++)
		close(_fds[i].fd);
}
