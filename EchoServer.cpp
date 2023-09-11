#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h> //ipv4주소를 10진수로 표현하기 위해 사용
#include <netinet/in.h> //ipv4의 ip주소와 포트번호를 나타내기 위해 사용
#include <sys/socket.h> //소켓통신 하기위해 필요

int main(void)
{
	//서버 소켓 생성 (접속대기 소켓 ServerSocket == socket handle)
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	/*
		socket함수의 프로토타입
		int socket(int domain, int type, int protocol);
		1. domain : 소켓의 주소체계를 지정. IPv4주소체계를 사용하려면 AF_INET, IPv6을 사용하려면 AF_INET6
			AF == address family -> L3가 IP Protocol이라는 뜻
		2. type : 소켓의 유형을 지정. SOCK_STREAM(TCP 소켓)과 SOCK_DGRAM(UDP)등을 사용
			SOCK_STREAM -> 소켓에서의 데이터 단위가 stream이다. == L4가 TCP
		3. protocol : 프로토콜을 지정. 주로 사용되는 값으로는 0또는 IPPROTO_TCP(TCP) 또는 IPPROTO_UDP(UDP)등이 있다.
	*/
	if (serverSocket == -1)
	{
		std::cerr << "Failed create ServerSocket" << std::endl;
		return 1;
	}

	//서버 주소 설정 (포트 바인딩)
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	/*
		htons(), htonl()
		보통 network 개발자들은 big endian을 사용해서 개발 -> 하지만 intel mac에서는 little endian을 사용
		따라서 big endian을 사용하는 것
		big endian을 사용한다면 굳이 함수를 호출할 필요가 없다.
	*/
	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cerr << "Failed bind ServerSocket" << std::endl;
		close(serverSocket);
		return 1;
	}
	/*
		bind()의 프로토타입
		int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
		sockfd : 바인딩할 소켓의 파일 디스크립터. 생성한 소켓의 파일 디스크립터를 전달해야 한다.
		addr : 바인딩할 주소 정보를 담고 있는 구조체의 포인터이다. 주로 struct sockaddr_in 또는 struct sockaddr_in6와 같은 구조체를 사용한다.
			이 구조체에는 IP주소와 포트 정보가 포함되어 있다.
		addrlen : addr구조체의 크기를 나타내는 변수이다.
	*/
	if (listen(serverSocket, SOMAXCONN) == -1)
	{
		std::cerr << "Failed Listen" << std::endl;
		close(serverSocket);
		return 1;
	}
	/*
		listen() 프로토타입
		int listen(int sockfd, int backlog);
		sockfd : 수신 대기 상태로 설정할 소켓의 파일 디스크립터이다.
			바인딩된 서버 소켓을 사용한다.
		backlog : 대기역에 저장할 연결 요청의 최대수를 나타내는 정수이다.
	*/

	std::cout << "Waiting 8080port" << std::endl;

	//클라이언트와 통신할 서버 통신 소켓을 여는 과정.
	struct sockaddr_in clientAddr= { 0 };
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (clientSocket == -1)
	{
		std::cerr << "Failed Accept client connection" << std::endl;
		close(serverSocket);
		return 1;
	}
	/*
		accept() 프로토타입
		int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		sockfd : 연결 요청을 수락할 서버 소켓의 파일 디스크립터. listen()로 대기상태로 설정된 서버 소켓을 사용
		addr : 클라이언트의 주소정보를 저장할 구조체의 포인터
		addrlen: addr 구조체의 크기를 나타내는 포인터.	
	*/
	char szBuffer[128] = { 0 };
	while (true)
	{
		int bytesRead = recv(clientSocket, szBuffer, sizeof(szBuffer), 0);
		std::cout << "from client: " << szBuffer << std::endl;
		/*
			소켓을 통해 데이터를 수신하는데 사용되는 네트워크 프로그래밍 함수
			ssize_t recv(int sockfd, void *buf, size_t len, int flags);
			sockfd : 데이터를 수신할 소켓의 파일 디스크립터이다.
			buf : 수신한 데이터를 저장할 버퍼의 포인터
			len : 버퍼의 크기를 나타내는 정수, 수신한 데이터를 저장하기에 충분한 크기여야한다.
			flags : 옵션 플래그를 나타내는 정수. 일반적으로 0으로 설정하며, 추가적인 옵션을 지정할 때 사용된다.
		*/
		if (bytesRead <= 0) {
			break;
		}
		send(clientSocket, szBuffer, 128, 0);
		memset(&szBuffer, 0, sizeof(szBuffer));
		/*
			상대방에게 데이터를 보내기위해 호출
			send() 프로토타입
			ssize_t send(int sockfd, const void *buf, size_t len, int flags);
			sockfd : 데이터를 전송할 소켓의 파일 디스크립터
			buf : 전송할 데이터가 저장된 버퍼의 포인터
			len : 데이터의 길이를 나타내는 정수. 버퍼에 있는 데이터 중 몇 바이트를 전송할지 지정한다.
			flags : 옵션 플래스를 나타내는 정수. 주로 0 으로 설정한다.
		*/
	}	
	close(clientSocket);
	close(serverSocket);

	return 0;
}