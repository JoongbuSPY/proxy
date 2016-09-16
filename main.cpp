#pragma comment(lib,"wsock32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>

int Get_IP_Addr(char *name, IN_ADDR *addr)
{
     HOSTENT * ptr = gethostbyname(name);

     if(ptr==NULL)
     {
          MessageBox(NULL,"IP주소 얻기 실패","Error!!",MB_OK);
          return NULL;
     }

     if(ptr->h_addrtype != AF_INET)
         return NULL;

     memcpy(addr, ptr->h_addr,ptr->h_length);

     return 4;
}

int main(int argc, char *argv[])
{
     int check;

     if(argc<2)
     {
          printf("파일이름.exe 포트번호\n");
          return 1;
     }


    WSADATA init_wsa;//윈속 초기화

    unsigned short server_port=atoi(argv[1]);

    if(WSAStartup(MAKEWORD(2,2),&init_wsa) != 0)
    {
        MessageBox(NULL,"윈속 초기화 실패","Error!!",MB_OK);
        return 1;
    }

     char host_name[20];
     HOSTENT* host_addr;

     if(gethostname( host_name, sizeof(host_name)) == 0 )
     {

        if((host_addr = gethostbyname(host_name)))
        {
            IN_ADDR my_addr;
            memcpy( &my_addr, host_addr->h_addr_list[0], 4 );
            printf( "내 컴퓨터 IP : %s \n", inet_ntoa(my_addr));
        }
    }


     SOCKET sock = socket(AF_INET,SOCK_STREAM,0);

     //1.소켓생성.

     if(sock== INVALID_SOCKET)
     {
         MessageBox(NULL,"소켓 생성 실패","Error!!",MB_OK);
         return 1;
     }

     //2. bind()
     SOCKADDR_IN server_addr;
     memset(&server_addr,0,sizeof(server_addr));
     server_addr.sin_family = AF_INET;
     server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
     server_addr.sin_port=htons(server_port);


     check=bind(sock,(SOCKADDR *)&server_addr,sizeof(server_addr));

     if(check==SOCKET_ERROR)
     {
        MessageBox(NULL,"Bind 실패","Error!!",MB_OK);
        return 1;
     }

     //listen

     check=listen(sock,SOMAXCONN);

     if(check==SOCKET_ERROR)
     {
        MessageBox(NULL,"Listen 실패","Error!!",MB_OK);
        return 1;
     }

     SOCKET client_sock;
     SOCKADDR_IN client_addr;
     char buf[65495];
     int len;

     while(true)
     {
        len=sizeof(client_addr);
        client_sock=accept(sock,(SOCKADDR *)&client_addr,&len);

        if(client_sock==INVALID_SOCKET)
        {
             MessageBox(NULL,"Accept 실패","Error!!",MB_OK);
             return 1;
        }

      //printf("클라이언트 IP: %s\n포트번호: %d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        while(true)
        {

            check=recv(client_sock,buf,65495,0);

            if(check==SOCKET_ERROR)
            {
                MessageBox(NULL,"Recv 실패","Error!!",MB_OK);
                return 1;
            }

            buf[check]='\0';

            printf("%s",buf);

            char *host=strstr(buf,"Host: ");
            char *domain=strtok(host,"\r\n");

            char *cut_host=domain+strlen("Host: ");

            host_addr = gethostbyname(cut_host);

            IN_ADDR domain_addr;

            int i = 0;

            while( host_addr->h_addr_list[i] )
            {
                memcpy( &domain_addr, host_addr->h_addr_list[i++], host_addr->h_length );
                printf("도메인 주소: %s\nIP주소: %s\n",cut_host,inet_ntoa(domain_addr));
            }

            SOCKET proxy_server_sock=socket(AF_INET,SOCK_STREAM,0);

            SOCKADDR_IN proxy_addr_in;
            proxy_addr_in.sin_family=AF_INET;
            proxy_addr_in.sin_port=htons(80);
            proxy_addr_in.sin_addr=*(in_addr *)(host_addr->h_addr_list[0]);

            if(connect(proxy_server_sock,(SOCKADDR *)&proxy_addr_in,sizeof(proxy_addr_in))==-1)
            {
                MessageBox(NULL,"Connect 실패","Error!!",MB_OK);
                return 1;
            }

            printf("\nConnected Server!!\n");

            int check2;

            check2=send(proxy_server_sock,buf,check,0);

            if(check2 == SOCKET_ERROR)
            {
                MessageBox(NULL,"Send 실패","Error!!",MB_OK);
                return 1;
            }

            check2=recv(proxy_server_sock,buf,sizeof(buf),0);
            buf[check2]='\0';

            printf("\n\n응답내용\n%s",buf);

            char *change;
            while(true)
            {
                change=strstr(buf,"hacking");

                if(change==NULL)
                    break;

                else
                    strncpy(change,"booming",strlen("booming"));
            }
            check2=send(client_sock,buf,check2,0);
        }

            closesocket(client_sock);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
