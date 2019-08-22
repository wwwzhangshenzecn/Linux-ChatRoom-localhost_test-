#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <set>
#include <map>

using namespace std;

#define BUFF_SIZE 2048
#define EPOLL_SIZE 2048

void ERROR_HANDLING(char *msg)
{
    cout << msg << endl;
    exit(1);
}

void server(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage : %s <IP>\n", argv[0]);
    }

    int serv_sock, clnt_sock;          // 服务器socket， 客户端socket
    map<int, struct sockaddr_in> user; // 存储当前连接的用户

    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_sz;
    int str_len, i; // str_len:接受的数据长度

    //epoll
    struct epoll_event *ep_events; // 存储发生变化的事件单元
    struct epoll_event event;
    int epfd, event_cnt; // epfd: epoll描述符， event_cnt： 发生改变事件数

    // 填写server地址信息
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (-1 == bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        ERROR_HANDLING("bind() ....");
    }
    if (-1 == listen(serv_sock, 10))
    {
        ERROR_HANDLING("listen() ....");
    }

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    event.data.fd = serv_sock;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while (1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, 10000);

        switch (event_cnt)
        {
        case 0:
            printf("Timeout...10s\n");
            break;
        case -1:
            ERROR_HANDLING("epoll_wait() error");
            break;
        default:
            // printf();
            for (int i = 0; i < event_cnt; i++)
            {
                if (ep_events[i].data.fd != serv_sock)
                {
                    //client
                    int client_sock = ep_events[i].data.fd;
                    struct sockaddr_in client_addr = user[client_sock];
                    char msg[BUFF_SIZE];
                    str_len = read(client_sock, msg, BUFF_SIZE);
                    string addr = inet_ntoa(client_addr.sin_addr);

                    string send_msg = "";

                    if (str_len == 0)
                    {
                        //exit
                        string exit_msg = "user : " + addr + " exit ...";
                        user.erase(client_sock);
                        send_msg = exit_msg;

                        epoll_ctl(epfd, EPOLL_CTL_DEL, client_sock, NULL);
                        close(client_sock);
                    }
                    else
                    {
                        // send_msg
                        msg[str_len] = 0;
                        send_msg = addr + " said: " + msg;
                    }
                    //分发消息
                    for (map<int, struct sockaddr_in>::iterator iter = user.begin(); iter != user.end(); iter++)
                    {
                        write(iter->first, send_msg.c_str(), send_msg.size());
                    }
                    printf("%s \n", send_msg.c_str());
                }
                else
                {
                    //server
                    clnt_addr_sz = sizeof(clnt_addr);
                    clnt_sock = accept(serv_sock,
                                       (struct sockaddr *)&clnt_addr, &clnt_addr_sz);

                    //注册
                    event.events = EPOLLIN;
                    event.data.fd = clnt_sock;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);


                    string addr = inet_ntoa(clnt_addr.sin_addr);
                    string msg = "User connected from " + addr;
                    for (map<int, struct sockaddr_in>::iterator iter = user.begin(); iter != user.end(); iter++)
                    {
                        write(iter->first, msg.c_str(), msg.size());
                    }
                    user.insert(make_pair(clnt_sock, clnt_addr));
                    printf("%s\n", msg.c_str());
                }
            }
        }
    }
    close(serv_sock);
    close(epfd);
}

int main(int argc, char *argv[])
{
    server(argc, argv);
    return 0;
}