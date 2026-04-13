#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket");
		return 1;
	}
	std::cout << "Waiting for connection on port 4242..." << std::endl;
	int setSocket;
	int opt = 1;
	setSocket = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (setSocket == -1) {
		perror("setsocket");
		return 1;
	}
	int bindSocket;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4242);
	addr.sin_addr.s_addr = INADDR_ANY;
	bindSocket = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (bindSocket == -1) {
		perror("bind");
		return 1;
	}
	int listenSock;
	listenSock = listen(sock, 10);
	if (listenSock == -1) {
		perror("listen");
		return 1;
	}
	int acceptSock;
	acceptSock = accept(sock, NULL, NULL);
	if (acceptSock == -1) {
		perror("accept");
		return 1;
	}

	int sendSock;
	std::string str = "Hello from my first socket!\n";
	sendSock = send(acceptSock, (const void *)str.c_str(), str.length(), 0);
	if (sendSock == -1) {
		perror("send");
		return 1;
	}
	close(sock);
}