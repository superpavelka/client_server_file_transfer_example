#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib,"wsock32.lib")
#include <stdio.h>

int main(int argc, char* argv[])
{
    const char* server_ip = "127.0.0.1";
    int server_port = 5555;
    const char* file_name = "test_client.mp4";
    int part_size = 1024;

    WSADATA wsa_data;
    if (WSAStartup(0x101, &wsa_data))
    {
        printf("WSAStartup error\n");
        system("pause");
        return -1;
    }
    if (wsa_data.wVersion != 0x101)
    {
        printf("WSA version error\n");
        system("pause");
        return -1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        printf("socket error\n");
        system("pause");
        return -1;
    }

    SOCKADDR_IN sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(server_port);
    sa.sin_addr.S_un.S_addr = inet_addr(server_ip);

    printf("wait for server...");
    while (true)
    {
        int connect_res = connect(s, (SOCKADDR*)&sa, sizeof(sa));
        if (!connect_res) break;

        Sleep(250);
    }

    printf("connected\n");

    printf("transfer...");

    FILE* f = fopen(file_name, "wb");
    if (!f)
    {
        printf("fopen error\n");
        system("pause");
        return -1;
    }

    long file_size = 0;

    if (recv(s, (char*)&file_size, sizeof(file_size), 0) != sizeof(file_size))
    {
        printf("recv error\n");
        system("pause");
        return -1;
    }

    char* buffer = new char[part_size];

    while (file_size)
    {
        int n = recv(s, buffer, part_size, 0);
        if (!n)
        {
            printf("disconnected\n");
            system("pause");
            return -1;
        }
        if (n == SOCKET_ERROR)
        {
            printf("recv error\n");
            system("pause");
            return -1;
        }
        file_size -= n;
        if (file_size < 0)
        {
            printf("file_size error\n");
            system("pause");
            return -1;
        }
        if (fwrite(buffer, 1, n, f) != n)
        {
            printf("fwrite error\n");
            system("pause");
            return -1;
        }
    }    

    delete[] buffer;

    fclose(f);

    closesocket(s);

    printf("complete\n");

    WSACleanup();

    system("pause");
    return 0;
}