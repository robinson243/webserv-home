#include "CgiHandler.hpp"
#include "EventLoop.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationConfig.hpp"
#include "Server.hpp"
#include "ServerConfig.hpp"
#include <csignal>
#include <iostream>

static EventLoop *g_eventloop = NULL;

static void signalHandler(int sig) {
	(void)sig;
	if (g_eventloop)
		g_eventloop->stop();
}

int main(int argc, char **argv) {
	std::string conf;
	if (argc != 2) {
		std::cerr << "Usage: ./webserv <configuration_file>\n";
		return (1);
	}
	conf.append(argv[1]);
	std::vector<ServerConfig> serverconfig;
	try {
		serverconfig = pars(conf);
		EventLoop server(serverconfig);
		signal(SIGINT, signalHandler);
		g_eventloop = &server;
		server.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << '\n';
		return (1);
	}
	return (0);
}
