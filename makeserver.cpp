#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h> //ipv4주소를 10진수로 표현하기 위해 사용
#include <netinet/in.h> //ipv4의 ip주소와 포트번호를 나타내기 위해 사용
#include <sys/event.h>
#include <sys/socket.h> //소켓통신 하기위해 필요
#include <fcntl.h>

int main(int ac, char **av)
{
	if (ac != 3)
		std::cerr << "plz put ./ircserv <port> <password>" << std::endl;

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (serverSocket == -1)
	{
		std::cerr << "Failed create ServerSocket" << std::endl;
		return 1;
	}
	int flags = fcntl(serverSocket, F_GETFL, 0);
	fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cerr << "Failed bind ServerSocket" << std::endl;
		close(serverSocket);
		return 1;
	}

	if (listen(serverSocket, SOMAXCONN) == -1)
	{
		std::cerr << "Failed Listen" << std::endl;
		close(serverSocket);
		return 1;
	}
	int kq = kqueue();
	if (kq < 0) {
		std::cerr << "Failef to create kqueue" << std::endl;
		close(serverSocket);
		return 1;
	}

	struct kevent events[10];
	struct kevent change;

	EV_SET(&change, serverSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);

	if (kevent(kq, &change, 1, NULL, 0, NULL) < 0) {
		std::cerr << "Failed to register server socket with kqueue" << std::endl;
		close(serverSocket);
		close(kq);
		return 1;
	}

	std::cout << "Waiting 8080port" << std::endl;

	while (true) {
		int nev = kevent(kq, NULL, 0, events, 10, NULL);
		if (nev < 0) {
			std::cerr << "kevent error" << std::endl;
			break;
		}

		for (int i = 0; i < nev; ++i) {
			if (events[i].ident == serverSocket) {
				int clientSocket = accept(serverSocket, NULL, NULL);
				if (clientSocket < 0) {
					std::cerr << "Accept failed" << std::endl;
				} else {
					flags = fcntl(clientSocket, F_GETFL, 0);
					fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
					EV_SET(&change, clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);

					if (kevent(kq, &change, 1, NULL, 0, NULL) < 0) {
						std::cerr << "Failed to register client socket with kqueue" << std::endl;
						close(clientSocket);
					}
				}
			}
			else {
				char buffer[1024];
				int bytesRead = read(events[i].ident, buffer, sizeof(buffer));
				if (bytesRead <= 0) {
					close(events[i].ident);
				} else {
					write(events[i].ident, buffer, bytesRead);
				}
			}
		}
	}

	close(serverSocket);
	close(kq);
	return 0;
}