#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.LIB")

constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

SOCKET g_client_s;
char buf[BUFSIZE];
WSABUF wsabuf[1];
WSAOVERLAPPED wsaover;
bool b_shutdown = false;

void error_display( const char* msg, int err_no )
{
	WCHAR* msgbuf = nullptr;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		, nullptr, err_no, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT )
		, (LPTSTR)msgbuf, 0, nullptr );
	std::cout << msg;
	std::wcout << msgbuf << std::endl;
	LocalFree( msgbuf );
}

void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

void CALLBACK send_callback(DWORD err_res,
	DWORD transfer_Size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	wsabuf[0].len = BUFSIZE;
	DWORD recv_flag = 0;
	ZeroMemory(&wsaover, sizeof(wsaover));
	WSARecv(g_client_s, wsabuf, 1, nullptr, &recv_flag, &wsaover, recv_callback);
}

void CALLBACK recv_callback(DWORD err_res,
	DWORD transfer_size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	if (0 == transfer_size) return;

	std::cout << "Message from CLIENT : ";
	for (DWORD i = 0; i < transfer_size; ++i) {
		std::cout << buf[i];
	}
	std::cout << std::endl;

	wsabuf[0].len = transfer_size;
	DWORD sent_size;
	WSASend(g_client_s, wsabuf, 1, nullptr, 0, &wsaover, send_callback);
}



int main( )
{
	std::wcout.imbue( std::locale( "korean" ) );
	WSAData wsa_data;
	auto res = WSAStartup( MAKEWORD( 2, 0 ), &wsa_data );
	SOCKET server_s = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED );
	SOCKADDR_IN addr_s;
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons( SERVER_PORT );
	addr_s.sin_addr.s_addr = htonl( ADDR_ANY );
	bind( server_s, reinterpret_cast<sockaddr*>( &addr_s ), sizeof( addr_s ) );
	listen( server_s, SOMAXCONN );
	
	sockaddr c_addr;
	int addr_size = static_cast<int>( sizeof( c_addr ) );
	while (false == b_shutdown) {
		g_client_s = WSAAccept(server_s, &c_addr, &addr_size, nullptr, 0);
		wsabuf[0].buf = buf;
		wsabuf[0].len = BUFSIZE;
		DWORD recv_flag = 0;
		int res = WSARecv(g_client_s, wsabuf, 1, nullptr, &recv_flag, &wsaover, recv_callback);
		if (0 != res) {
			auto err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("WSARecv Error : ", err_no);
		}
	}
	//while (true) SleepEx(0, true);
	closesocket( g_client_s );
	WSACleanup( );
}