#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib,"wsock32.lib")
#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char* argv[])
{
    int server_port = 5555;
    const char* file_name = "test_server.mp4";
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
    sa.sin_addr.S_un.S_addr = INADDR_ANY;

    if (bind(s, (SOCKADDR*)&sa, sizeof(sa)) == SOCKET_ERROR)
    {
        printf("bind error\n");
        system("pause");
        return -1;
    }
    if (listen(s, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("listen error\n");
        system("pause");
        return -1;
    }

    printf("wait for client...");

    int select_res = 0;
    while (true)
    {
        fd_set s_set;
        FD_ZERO(&s_set);
        FD_SET(s, &s_set);
        timeval timeout = { 0, 0 };
        select_res = select(s + 1, &s_set, 0, 0, &timeout);
        if (select_res) break;

        Sleep(250);
    }
    if (select_res == SOCKET_ERROR)
    {
        printf("select error\n");
        system("pause");
        return -1;
    }  

    SOCKET ns;

    SOCKADDR_IN nsa;
    int sizeof_nsa = sizeof(nsa);

    ns = accept(s, (SOCKADDR*)&nsa, &sizeof_nsa);
    if (ns == INVALID_SOCKET)
    {
        printf("accept error\n");
        system("pause");
        return -1;
    }

    printf("connected\n");

    printf("transfer...");

    struct stat si;
    if (stat(file_name, &si))
    {
        printf("stat error\n");
        system("pause");
        return -1;
    }

    if (send(ns, (char*)&si.st_size, sizeof(si.st_size), 0) == SOCKET_ERROR)
    {
        printf("send error\n");
        system("pause");
        return -1;
    }

    int parts_count = si.st_size / part_size;
    int last_part_size = si.st_size % part_size;

    FILE* f = fopen(file_name, "rb");
    if (!f)
    {
        printf("fopen error\n");
        system("pause");
        return -1;
    }

    char* buffer = new char[part_size];

    for (int i = 0; i < parts_count; i++)
    {
        if (fread(buffer, 1, part_size, f) != part_size)
        {
            printf("fread error\n");
            system("pause");
            return -1;
        }
        if (send(ns, buffer, part_size, 0) == SOCKET_ERROR)
        {
            printf("send error\n");
            system("pause");
            return -1;
        }
    }

    if (last_part_size)
    {
        if (fread(buffer, 1, last_part_size, f) != last_part_size)
        {
            printf("fread error\n");
            system("pause");
            return -1;
        }
        if (send(ns, buffer, last_part_size, 0) == SOCKET_ERROR)
        {
            printf("send error\n");
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