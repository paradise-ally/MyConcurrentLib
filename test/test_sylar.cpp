#include "sylar.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <stack>

static int sock_listen_fd = -1;
void test_accept();
void error(char* msg)
{
	perror(msg);
	printf("erreur...\n");
	exit(1);
}

void watch_io_read() {
	sylarr::IOManager::GetThis()->
		addEvent(sock_listen_fd, sylarr::IOManager::READ, test_accept);
}

void test_accept() {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t len = sizeof(addr);
	int fd = accept(sock_listen_fd, (struct sockaddr*)&addr, &len);
	if (fd < 0) {
		std::cout << "fd = " << fd << "accept false" << std::endl;
	}
	else {
		fcntl(fd, F_SETFL, O_NONBLOCK);
		sylarr::IOManager::GetThis()->
			addEvent(fd, sylarr::IOManager::READ, [fd]() {
			char buffer[1024];
			memset(buffer, 0, sizeof(buffer));
			while (true) {
				int ret = recv(fd, buffer, sizeof(buffer), 0);
				if (ret > 0) {
					ret = send(fd, buffer, ret, 0);
				}
				if (ret <= 0) {
					if (errno == EAGAIN) continue;
					close(fd);
					break;
				}
			}
				});
	}
	sylarr::IOManager::GetThis()->schedule(watch_io_read);
}

void test_iomanager() {
	int portno = 8080;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len = sizeof(client_addr);
	// setup socket
	sock_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_listen_fd < 0) {
		error("Error creating socket..\n");
	}

	int yes = 1;
	// lose the pesky "address already in use" error message
	setsockopt(sock_listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portno);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// bind socket and listen for connections
	if (bind(sock_listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		error("Error binding socket..\n");
	if (listen(sock_listen_fd, 1024) < 0) {
		error("Error listening..\n");
	}
	printf("epoll echo server listening for connections on port: %d\n", portno);
	fcntl(sock_listen_fd, F_SETFL, O_NONBLOCK);
	sylarr::IOManager iom;
	iom.addEvent(sock_listen_fd, sylarr::IOManager::READ, test_accept);
}

int main(int argc, char* argv[]) {
	test_iomanager();
	return 0;
}
