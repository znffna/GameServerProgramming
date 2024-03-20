#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.LIB")

constexpr char SERVER_ADDR[ ] = "127.0.0.1";
constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

SOCKET g_server_s;
char buf[BUFSIZE];
WSABUF wsabuf[1];
WSAOVERLAPPED wsaover;
bool b_logout = false;


void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void send_message_to_server();


void send_message_to_server() {
	std::cout << "Enter Message : ";
	std::cin.getline(buf, BUFSIZE);
	if (buf[0] == 0) {
		b_logout = true;
		return;
	}

	wsabuf[0].buf = buf;
	wsabuf[0].len = static_cast<int>(strlen(buf) + 1);;
	DWORD sent_size;
	ZeroMemory(&wsaover, sizeof(wsaover));
	WSASend(g_server_s, wsabuf, 1, &sent_size, 0, &wsaover, send_callback);
}

void CALLBACK send_callback(DWORD err_res,
	DWORD transfer_Size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	wsabuf[0].len = BUFSIZE;
	DWORD recv_flag = 0;
	ZeroMemory(&wsaover, sizeof(wsaover));
	WSARecv(g_server_s, wsabuf, 1, nullptr, &recv_flag, &wsaover, recv_callback);
}

void CALLBACK recv_callback(DWORD err_res,
	DWORD transfer_Size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	std::cout << "Message from Server : ";
	for (DWORD i = 0; i < transfer_Size; ++i) {
		std::cout << buf[i];
	}
	std::cout << std::endl;

	send_message_to_server();
}

int main( )
{
	std::wcout.imbue( std::locale( "korean" ) );
	WSAData wsa_data;
	WSAStartup( MAKEWORD( 2, 0 ), &wsa_data );
	g_server_s = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED );
	SOCKADDR_IN addr_s;
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons( SERVER_PORT );
	inet_pton( AF_INET, SERVER_ADDR, &addr_s.sin_addr );
	connect(g_server_s, reinterpret_cast<sockaddr*>( &addr_s ), sizeof( addr_s ) );

	send_message_to_server();
	while (false == b_logout) {
		SleepEx(0, true);		// 콜백을 실행하라는 호출
	}

	closesocket(g_server_s);
	WSACleanup( );
}