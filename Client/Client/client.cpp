#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")

constexpr char SERVER_ADDR[] = "127.0.0.1";
constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

SOCKET g_server_s;
char buf[BUFSIZE];
WSABUF wsabuf[1];
WSAOVERLAPPED wsaover;
bool b_logout = false;

void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void send_message_to_server();

void CALLBACK recv_callback(DWORD err_res,
	DWORD r_size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	std::cout << "Message from Server :\n";
	if (r_size == 0) {
		b_logout = true;
		return;
	}

	int p_size = 0;
	while (p_size < r_size) {
		char m_size = buf[0 + p_size];
		int c_id = buf[1 + p_size];
		std::cout << "Clien[" << c_id << "] sent : ";
		for (char i = 0; i < m_size; ++i)
			std::cout << buf[i + 2 + p_size];
		std::cout << std::endl;
		p_size += m_size;
	}

	send_message_to_server();
}

void CALLBACK send_callback(DWORD err_res,
	DWORD transfer_size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	wsabuf[0].len = BUFSIZE;
	DWORD recv_flag = 0;
	ZeroMemory(&wsaover, sizeof(wsaover));
	WSARecv(g_server_s, wsabuf, 1, nullptr, &recv_flag, &wsaover, recv_callback);
}

void send_message_to_server()
{
	std::cout << "Enter Message : ";
	std::cin.getline(buf, BUFSIZE);
	if (buf[0] == 0) {
		b_logout = true;
		return;
	}
	wsabuf[0].buf = buf;
	wsabuf[0].len = static_cast<int>(strlen(buf)) + 1;
	DWORD sent_size;
	ZeroMemory(&wsaover, sizeof(wsaover));
	WSASend(g_server_s, wsabuf, 1, &sent_size, 0, &wsaover, send_callback);
}

int main()
{
	std::wcout.imbue(std::locale("korean"));
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 0), &wsa_data);
	g_server_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN addr_s;
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &addr_s.sin_addr);
	connect(g_server_s, reinterpret_cast<sockaddr*>(&addr_s), sizeof(addr_s));
	send_message_to_server();
	while (false == b_logout)
		SleepEx(0, true);
	closesocket(g_server_s);
	WSACleanup();
}
