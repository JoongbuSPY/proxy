#pragma comment(lib,"wsock32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>


DWORD WINAPI p_th(void *data)
{
	SOCKET th_sock = (SOCKET)data;
	int t_send,t_recv,check,i=0,check2;
	char th_buf[65495];
	HOSTENT* host_addr;//ip주소를 담는 HOSTENT라는 구조체변수 host_addr 선언.
	ZeroMemory(&th_buf,sizeof(th_buf));
	while(check = recv(th_sock,th_buf,65495,0))
	{
     	if(check == SOCKET_ERROR)
			break;
		th_buf[check] = '\0';
		char *host = strstr(th_buf, "Host: ");  
		char *domain = strtok(host, "\r\n");
		char *cut_host = domain + strlen("Host: ");
		host_addr = gethostbyname(cut_host);
		IN_ADDR domain_addr;
  
		memcpy(&domain_addr, host_addr->h_addr_list[i], host_addr->h_length);
		printf("도메인 주소: %s\nIP주소: %s\n", cut_host, inet_ntoa(domain_addr));
		SOCKET proxy_server_sock = socket(AF_INET, SOCK_STREAM, 0);
		SOCKADDR_IN proxy_addr_in;
		proxy_addr_in.sin_family = AF_INET;
		proxy_addr_in.sin_port = htons(80);
		proxy_addr_in.sin_addr = *(in_addr *)(host_addr->h_addr_list[0]);

		if (connect(proxy_server_sock, (SOCKADDR *)&proxy_addr_in, sizeof(proxy_addr_in)) == -1)
		{
			MessageBox(NULL, "Connect 실패", "Error!!", MB_OK);
			return 1;
		}

		printf("\nConnected Server!!\n");
  
		check2 = send(proxy_server_sock, th_buf, check, 0);
  
		if (check2 == SOCKET_ERROR)
		{
			printf("어디 1\n");
			break;
		} 

		check2 = recv(proxy_server_sock, th_buf, sizeof(th_buf), 0);
  
		if (check2 == SOCKET_ERROR)
		{
			MessageBox(NULL, "recv 실패", "Error!!", MB_OK);
			return 1;
		}

		th_buf[check2] = '\0';
		printf("\n\n응답내용\n%s", th_buf);

		char *change;

		while (true)
		{
			change = strstr(th_buf, "hacking");

			if (change == NULL)
			{
				printf("어디 2\n");
				break;
			}
			else
				strncpy(change, "booming", strlen("booming"));
		}

		check2=send(th_sock, th_buf, check2, 0);
  
		if (check2 == SOCKET_ERROR)
		{
			printf("어디 3\n");
			break;
		}
  
		ZeroMemory(&th_buf,sizeof(th_buf));
	}
 
	printf("쓰레드안\n\n\n");

	closesocket(th_sock); 
	return 0;
}

int main(int argc, char *argv[])
{
	int check;
	if (argc<2)
	{
		printf("파일이름.exe 포트번호\n");
		return 1;
	}

	WSADATA init_wsa;//윈속 초기화
	unsigned short server_port = atoi(argv[1]);//포트번호 초기화
	if (WSAStartup(MAKEWORD(2, 2), &init_wsa) != 0)
	{
		MessageBox(NULL, "윈속 초기화 실패", "Error!!", MB_OK);
		return 1;
	}


	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	//1.소켓생성.
	if (sock == INVALID_SOCKET)
	{
		MessageBox(NULL, "소켓 생성 실패", "Error!!", MB_OK);
		return 1;
	}
	//2. bind()
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(server_port);

	check = bind(sock, (SOCKADDR *)&server_addr, sizeof(server_addr));
	if (check == SOCKET_ERROR)
	{
		MessageBox(NULL, "Bind 실패", "Error!!", MB_OK);
		return 1;
	}
	//3. listen
	check = listen(sock, SOMAXCONN);
	if (check == SOCKET_ERROR)
	{
		MessageBox(NULL, "Listen 실패", "Error!!", MB_OK);
		return 1;
	}
	SOCKET client_sock;
	SOCKADDR_IN client_addr;
	int len;
	HANDLE th;
	//4.accept
	while(true)
	{
		len = sizeof(client_addr);
		client_sock = accept(sock, (SOCKADDR *)&client_addr, &len);
		if (client_sock == INVALID_SOCKET)
		{
			MessageBox(NULL, "Accept 실패", "Error!!", MB_OK);
			return 1;
		}
		th = CreateThread(NULL,0,p_th,(void *)client_sock,0,NULL);
	}

 
	printf("쓰레드밖\n\n\n");
	closesocket(sock);
	WSACleanup();

	return 0;
}
