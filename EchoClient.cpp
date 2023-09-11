#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main()
{
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == -1)
	{
		std::cerr << "Failed create Clent Socket" << std::endl;
		return 1;
	}

	struct sockaddr_in serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr.s_addr) <= 0)
	{
		std::cout << "Error: Invalid server ID: " << serverAddr.sin_addr.s_addr << std::endl;
		return 1;
	}
	/*
		inet_pton() 의 프로토타입
		int inet_pton(int af, const char *src, void *dst);
		af : 주소 채계를 나타내는 매개변수
		src : 변환할 IP주소 문자열
		dst : 변환된 이진 형태의 IP주소를 저장할 버퍼의 포인터
		성공할경우 1, 실패할경우 0또는 -1

	*/
	if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cerr << "Failed conneting Server" << std::endl;
		close(clientSocket);
		return 1;
	}
	/*
		connect() 의 프로토타입
		int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
		sockfd : 연결을 시도할 클라이언트 소켓의 파일 디스크립터
		addr: 연결할 서버의 주소 정보가 담신 struct sockaddr 구조체의 포인터
		addrlen : addr 구조체의 크기를 나타내는 변수.
	*/
	char szBuffer[128] = { 0 };
	while (true) {
		std::cout << "Message: ";
		std::cin.getline(szBuffer, sizeof(szBuffer));
		// std::cin >> szBuffer;
		if (strcmp(szBuffer, "EXIT") == 0)
			break;
		send(clientSocket, szBuffer, strlen(szBuffer) + 1, 0);
		memset(szBuffer, 0, sizeof(szBuffer));
		int bytesRead = recv(clientSocket, szBuffer, sizeof(szBuffer), 0);
		if (bytesRead <= 0) {
			std::cerr << "Failed receiving a response from the server" << std::endl;
			break;
		}
		std::cout << "From Server: " << szBuffer << std::endl;
	}
	std::cout << "close server" << std::endl;
	shutdown(clientSocket, SHUT_RDWR);
	close(clientSocket);
	return (0);
}