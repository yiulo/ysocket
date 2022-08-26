#pragma once
#ifndef __WIN_SOCKET__
#define __WIN_SOCKET__

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#elif __linux__
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif // _WIN32 | __linux__

#include <stdio.h>

typedef struct Yaddr4 {
	sockaddr addr = { 0 };
	int nAddrlen = 16;
} Yaddr4;

#ifdef _WIN32

typedef struct ipv4 {
	u_char s[4];
	static ipv4 u(u_char* ip) {
		return { ip[0],ip[1],ip[2],ip[3] };
	}
	static ipv4 l(long ip) {
		return { (u_char)(&ip)[3],(u_char)(&ip)[2],(u_char)(&ip)[1],(u_char)(&ip)[0] };
	}
	static ipv4 str(const char* ip) {
		long _l = inet_addr(ip);
		return { (u_char)(_l & 0xff), (u_char)(_l >> 8 & 0xff), (u_char)(_l >> 16 & 0xff), (u_char)(_l >> 24 & 0xff) };
	}
} ipv4;

typedef struct Ysocket4 {
	SOCKET sock;
	sockaddr_in sin;
	WSADATA wsadata;
	int isRun = 0;

	Yaddr4 createServer(int port, WORD sockVision = MAKEWORD(2, 2)) {
		if (WSAStartup(sockVision, &wsadata) != 0)
		{
			printf("WSA初始化失败\n");
			return { 0 };
		}

		//创建套接字
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		printf("sock: %x,%d\n", sock, sock);
		if (sock == INVALID_SOCKET)
		{
			printf("socket服务器创建失败(%d)\n", sock);
			return { 0 };
		}

		//绑定IP和端口
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.S_un.S_addr = INADDR_ANY;
		if (bind(sock, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
		{
			printf("绑定IP和端口\n");
			return { 0 };
		}
		sin.sin_addr.S_un.S_addr = 0x7f000001;

		return { *((SOCKADDR*)&sin) , sizeof(sin) };
	}
	Yaddr4 createClient(ipv4 ip, int port, WORD sockVision = MAKEWORD(2, 2)) {
		Yaddr4 add4;
		WSADATA wsadata;
		if (WSAStartup(sockVision, &wsadata) != 0)
		{
			printf("WSA初始化失败\n");
			return { 0 };
		}

		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sock == INVALID_SOCKET)
		{
			printf("socket客户端创建失败\n");
			return{ 0 };
		}
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.S_un.S_un_b = { ip.s[0], ip.s[1], ip.s[2], ip.s[3] };
		return { *((SOCKADDR*)&sin) , sizeof(sin) };
	}

	// 统一创建接口
	Yaddr4 create(int port, WORD sockVision = MAKEWORD(2, 2)) {
		return createServer(port, sockVision);
	}
	Yaddr4 create(ipv4 ip, int port, WORD sockVision = MAKEWORD(2, 2)) {
		return createClient(ip, port, sockVision);
	}
	Yaddr4 create(u_char* ip_char, int port, WORD sockVision = MAKEWORD(2, 2)) {
		return createClient(ipv4::u(ip_char), port, sockVision);
	}
	Yaddr4 create(long ip_long, int port, WORD sockVision = MAKEWORD(2, 2)) {
		return createClient(ipv4::l(ip_long), port, sockVision);
	}
	Yaddr4 create(const char* ip_str, int port, WORD sockVision = MAKEWORD(2, 2)) {
		return createClient(ipv4::str(ip_str), port, sockVision);
	}

	// 发送数据
	int send(char* data, int len, SOCKADDR* addr, int nAddrlen = sizeof(SOCKADDR)) {
		return sendto(sock, data, len, 0, addr, nAddrlen);
	}
	int send(char* data, int len, Yaddr4* addr) {
		return sendto(sock, data, len, 0, &addr->addr, addr->nAddrlen);
	}
	int send(char* data, int len) {
		return sendto(sock, data, len, 0, (SOCKADDR*)&sin, 16);
	}
#ifdef __INET_DATA_PACKAGE__
	int send(inetDataOutPackage idp, SOCKADDR* addr, int nAddrlen = sizeof(SOCKADDR)) {
		int o = 0;
		char* data;
		int len;
		for (int i = 0; i < idp.qty; i++) {
			idp.getSharding(i, &data, &len);
			o += sendto(sock, data, len, 0, addr, nAddrlen);
		}
		return o;
	}
	int send(inetDataOutPackage idp, Yaddr4* addr) {
		int o = 0;
		char* data;
		int len;
		for (int i = 0; i < idp.qty; i++) {
			idp.getSharding(i, &data, &len);
			o += sendto(sock, data, len, 0, &addr->addr, addr->nAddrlen);
		}
		return o;
	}
	int send(inetDataOutPackage idp) {
		int o = 0;
		char* data;
		int len;
		for (int i = 0; i < idp.qty; i++) {
			idp.getSharding(i, &data, &len);
			o += sendto(sock, data, len, 0, (SOCKADDR*)&sin, 16);
		}
		return o;
	}
	// n 每毫秒发送次数
	int send(inetDataOutPackage idp, long long send_qty, SOCKADDR* addr, int nAddrlen = sizeof(SOCKADDR)) {
		int o = 0;
		char* data;
		int len;
		long long i = 0;
		while (1)
		{
			for (int ii = 0; ii < send_qty; ii++)
			{
				if (i >= idp.qty) { break; }
				idp.getSharding(i, &data, &len);
				o += sendto(sock, data, len, 0, addr, nAddrlen);
				i++;
			}
			if (i >= idp.qty) { break; }
			Sleep(1);
		}
		return o;
	}
#endif // __INET_DATA_PACKAGE__

	// 接收数据
	int receive(char* data, int len, SOCKADDR* addr, int* nAddrlen) {
		return recvfrom(sock, data, len, 0, addr, nAddrlen);
	}
	int receive(char* data, int len, Yaddr4* addr) {
		return recvfrom(sock, data, len, 0, &addr->addr, &addr->nAddrlen);
	}
	int receive(char* data, int len) {
		return recvfrom(sock, data, len, 0, 0, 0);
	}

	// 设置是否阻塞 默认下开启阻塞
	int setClog(int tf) {
		return ioctlsocket(sock, FIONBIO, (u_long FAR*) & tf);
	}

	// 回收、关闭端口使用
	int closeSock() {
		closesocket(sock);
		return 0;
	}
	int allEndSock() {
		closeSock();
		WSACleanup();
		return 0;
	}
} Ysocket4;

#elif __linux__

typedef struct ipv4 {
	u_char s[4];
	static ipv4 u(u_char* ip) {
		return { ip[0], ip[1], ip[2], ip[3] };
	}
	static ipv4 l(long ip) {
		return u((u_char*)&ip);
	}
	static ipv4 str(const char* ip) {
		return l(inet_addr(ip));
	}
} ipv4;

typedef struct Ysocket4 {
	int sock;
	sockaddr_in sin;
	int isRun = 0;

	Yaddr4 createServer(int port) {
		Yaddr4 bad;

		//创建套接字
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock == ~0)
		{
			printf("socket服务器创建失败(%d)\n", sock);
			return bad;
		}

		//绑定IP和端口
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = INADDR_ANY;
		if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)) == -1)
		{
			printf("绑定IP和端口失败\n");
			return bad;
		}
		sin.sin_addr.s_addr = 0x7f000001;

		bad.addr = *((sockaddr*)&sin);
		return bad;
	}
	Yaddr4 createClient(ipv4 ip, int port) {
		Yaddr4 bad;

		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sock == ~0)
		{
			printf("socket客户端创建失败\n");
			return bad;
		}
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = *(long*)&ip.s;
		bad.addr = *((sockaddr*)&sin);
		return bad;
	}

	// 统一创建接口
	Yaddr4 create(int port) {
		return createServer(port);
	}
	Yaddr4 create(ipv4 ip, int port) {
		return createClient(ip, port);
	}
	Yaddr4 create(u_char* ip_char, int port) {
		return createClient(ipv4::u(ip_char), port);
	}
	Yaddr4 create(long ip_long, int port) {
		return createClient(ipv4::l(ip_long), port);
	}
	Yaddr4 create(const char* ip_str, int port) {
		return createClient(ipv4::str(ip_str), port);
	}

	// 发送数据
	int send(char* data, int len, sockaddr* addr, int nAddrlen = sizeof(sockaddr)) {
		return sendto(sock, data, len, 0, addr, nAddrlen);
	}
	int send(char* data, int len, Yaddr4* addr) {
		return sendto(sock, data, len, 0, &addr->addr, addr->nAddrlen);
	}
	int send(char* data, int len) {
		return sendto(sock, data, len, 0, (sockaddr*)&sin, 16);
	}
#ifdef __INET_DATA_PACKAGE__
	int send(inetDataOutPackage idp, sockaddr* addr, int nAddrlen = sizeof(sockaddr)) {
		int o = 0;
		char* data;
		int len;
		for (int i = 0; i < idp.qty; i++) {
			idp.getSharding(i, &data, &len);
			o += sendto(sock, data, len, 0, addr, nAddrlen);
		}
		return o;
	}
	int send(inetDataOutPackage idp, Yaddr4* addr) {
		int o = 0;
		char* data;
		int len;
		for (int i = 0; i < idp.qty; i++) {
			idp.getSharding(i, &data, &len);
			o += sendto(sock, data, len, 0, &addr->addr, addr->nAddrlen);
		}
		return o;
	}
	int send(inetDataOutPackage idp) {
		int o = 0;
		char* data;
		int len;
		for (int i = 0; i < idp.qty; i++) {
			idp.getSharding(i, &data, &len);
			o += sendto(sock, data, len, 0, (sockaddr*)&sin, 16);
		}
		return o;
	}
	// n 每毫秒发送次数
	int send(inetDataOutPackage idp, long long send_qty, sockaddr* addr, int nAddrlen = sizeof(sockaddr)) {
		int o = 0;
		char* data;
		int len;
		long long i = 0;
		while (1)
		{
			for (int ii = 0; ii < send_qty; ii++)
			{
				if (i >= idp.qty) { break; }
				idp.getSharding(i, &data, &len);
				o += sendto(sock, data, len, 0, addr, nAddrlen);
				i++;
			}
			if (i >= idp.qty) { break; }
			sleep(1);
		}
		return o;
	}
#endif // __INET_DATA_PACKAGE__

	// 接收数据
	int receive(char* data, int len, sockaddr* addr, int* nAddrlen) {
		return recvfrom(sock, data, len, 0, addr, (socklen_t*)nAddrlen);
	}
	int receive(char* data, int len, Yaddr4* addr) {
		return recvfrom(sock, data, len, 0, &addr->addr, (socklen_t*)&addr->nAddrlen);
	}
	int receive(char* data, int len) {
		return recvfrom(sock, data, len, 0, 0, 0);
	}

	// 设置是否阻塞 默认下开启阻塞
	int setClog(int tf) {
		//return ioctlsocket(sock, FIONBIO, (u_long FAR*) & tf);
		return 0;
	}

	// 回收、关闭端口使用
	int closeSock() {
		close(sock);
		return 0;
	}
	int allEndSock() {
		return closeSock();
	}
} Ysocket4;

#endif // _WIN32


#endif // !__WIN_SOCKET__
