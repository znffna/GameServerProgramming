#include <iostream>
#include <unordered_map>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.LIB")

constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

char g_session_id = 0;
void error_display(const char* msg, int err_no)
{
	WCHAR* msgbuf = nullptr;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		, nullptr, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
		, (LPTSTR)msgbuf, 0, nullptr);
	std::cout << msg;
	std::wcout << msgbuf << std::endl;
	LocalFree(msgbuf);
}

void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

class SESSION;
std::unordered_map<LPWSAOVERLAPPED, SESSION> players;		// 컨테이너 생성


class EXP_OVER {
public:
	WSAOVERLAPPED wsaover;
	WSABUF wsabuf[1];
	char buf[BUFSIZE];
	
	EXP_OVER(char s_id, char* mess, int m_size)
	{
		ZeroMemory(&wsaover, sizeof(wsaover));
		wsabuf[0].buf = buf;
		wsabuf[0].len = m_size + 2;	// 사이즈 + id + 메시지
		buf[0] = m_size + 2;
		buf[1] = s_id;
		memcpy(buf + 2, mess, m_size);
	}
};

class SESSION {
	char s_id_;
	SOCKET client_s_;	// m_client_s
	char buf[BUFSIZE];
	WSABUF wsabuf[1];
	WSAOVERLAPPED *wsaover;
public:
	SESSION()
	{
		std::cout << "ERROR\n";
		exit(-1);
	}

	SESSION(SOCKET s, WSAOVERLAPPED *over) : client_s_(s), wsaover(over)
	{
		s_id_ = g_session_id++;
		wsabuf[0].buf = buf;
		wsabuf[0].len = BUFSIZE;
	}

	~SESSION()
	{
		closesocket(client_s_);
		delete wsaover;
	}

	void do_recv() 
	{
		ZeroMemory(wsaover, sizeof(*wsaover));
		DWORD recv_flag = 0;
		int res = WSARecv(client_s_, wsabuf, 1, nullptr, &recv_flag, wsaover, recv_callback);
		if (0 != res) {
			auto err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("WSARecv Error : ", err_no);
		}
	}

	void do_send(char s_id, char* buf, int m_size)
	{
		// 매개변수의 buf는 recv의 buf. 따라서 send의 buf는 복사를 해야한다.
		// -> EXP_OVER에서 수행.
		auto b = new EXP_OVER(s_id, buf, m_size);
		WSASend(client_s_, b->wsabuf, 1, nullptr, 0, &b->wsaover, send_callback);
		
	}


	void do_recv_callback(DWORD transfer_size)
	{
		// 화면에 출력
		if (0 == transfer_size) return;
		std::cout << "Message from CLIENT[" << static_cast<int>(s_id_) << "] : ";

		for (DWORD i = 0; i < transfer_size; ++i) {
			std::cout << buf[i];
		}
		std::cout << std::endl;

		// 모든 세션에게 메시지를 보낸다.
		for (auto& ss : players)
			ss.second.do_send(s_id_, buf, transfer_size);
		do_recv();
	}

};


bool b_shutdown = false;


void CALLBACK send_callback(DWORD err_res,
	DWORD transfer_Size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	// 보내기 위해 복사된 EXP_OVER를 반환한다.
	auto b = reinterpret_cast<EXP_OVER*> (p_wsaover);
	delete b;
}

void CALLBACK recv_callback(DWORD err_res,
	DWORD transfer_size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	players[p_wsaover].do_recv_callback(transfer_size);
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
		SOCKET client_s = WSAAccept(server_s, &c_addr, &addr_size, nullptr, 0);
		LPWSAOVERLAPPED p_over = new WSAOVERLAPPED;
		players.try_emplace(p_over, client_s, p_over);
		players[p_over].do_recv();
	}

	players.clear();
	closesocket(server_s);
	WSACleanup( );
}