#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.LIB")

constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

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

int main( )
{
	std::wcout.imbue( std::locale( "korean" ) );
	WSAData wsa_data;
	WSAStartup( MAKEWORD( 2, 0 ), &wsa_data );
	SOCKET server_s = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0 );
	SOCKADDR_IN addr_s;
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons( SERVER_PORT );
	addr_s.sin_addr.s_addr = htonl( ADDR_ANY );
	bind( server_s, reinterpret_cast<sockaddr*>( &addr_s ), sizeof( addr_s ) );
	listen( server_s, SOMAXCONN );
	
	sockaddr c_addr;
	int addr_size = static_cast<int>( sizeof( c_addr ) );
	SOCKET client_s = WSAAccept( server_s, &c_addr, &addr_size, nullptr, 0 );

	while ( true ) {
		char buf[ BUFSIZE ];
		WSABUF wsabuf[ 1 ];
		wsabuf[ 0 ].buf = buf;
		wsabuf[ 0 ].len = BUFSIZE;
		DWORD recv_size;
		DWORD recv_flag = 0;
		WSARecv( client_s, wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr );
		if ( recv_size == 0 ) break;
		
		std::cout << "Message from CLIENT : ";
		for ( DWORD i = 0; i < recv_size; ++i ) {
			std::cout << buf[ i ];
		}
		std::cout << std::endl;

		wsabuf[ 0 ].len = recv_size;
		DWORD sent_size;
		WSASend( client_s, wsabuf, 1, &sent_size, 0, nullptr, nullptr );
	}

	closesocket( client_s );
	WSACleanup( );
}