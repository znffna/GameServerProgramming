#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.LIB")

constexpr char SERVER_ADDR[ ] = "127.0.0.1";
constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

int main( )
{
	std::wcout.imbue( std::locale( "korean" ) );
	WSAData wsa_data;
	WSAStartup( MAKEWORD( 2, 0 ), &wsa_data );
	SOCKET server_s = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0 );
	SOCKADDR_IN addr_s;
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons( SERVER_PORT );
	inet_pton( AF_INET, SERVER_ADDR, &addr_s.sin_addr );
	connect( server_s, reinterpret_cast<sockaddr*>( &addr_s ), sizeof( addr_s ) );

	while ( true ) {
		char buf[ BUFSIZE ];
		std::cout << "Enter Message : ";
		std::cin.getline( buf, BUFSIZE );
		if ( buf[ 0 ] == 0 ) break;

		WSABUF wsabuf[ 1 ];
		wsabuf[ 0 ].buf = buf;
		wsabuf[ 0 ].len = static_cast<int>( strlen( buf ) + 1 );;
		DWORD sent_size;
		WSASend( server_s, wsabuf, 1, &sent_size, 0, nullptr, nullptr );

		wsabuf[ 0 ].len = BUFSIZE;
		DWORD recv_size;
		DWORD recv_flag = 0;
		WSARecv( server_s, wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr );

		std::cout << "Message from Server : ";
		for ( DWORD i = 0; i < recv_size; ++i ) {
			std::cout << buf[ i ];
		}
		std::cout << std::endl;
	}

	closesocket( server_s );
	WSACleanup( );
}