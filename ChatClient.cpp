#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include<sys/wait.h>


#define INADDR_MY "127.0.0.1"
using namespace std;
#define BUFF_SIZE 2048

void ErrorHanling(char *s)
{
    cout << s << endl;
    exit(1);
}

int main(int argc, char *argv[])
{

    int sock;
    struct sockaddr_in serv_addr;

    char message[BUFF_SIZE];
    int str_len;

    if (argc != 3)
    {
        cout << "usgae :" << argv[0] << "<IP> <PORT>" << endl;
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        ErrorHanling("socket() error.");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        ErrorHanling("connect() error");
    }

    int pid = fork();

    if (pid !=0)
    {   

        int status;
        while (!waitpid(-1, &status, WNOHANG ))
        {
            // read
            string recv_msg = "";
            while (0 != (str_len = read(sock, message, BUFF_SIZE)))
            {
                message[str_len] = 0;
                cout << message << endl;
            }

        }

        if(WIFEXITED(status)){
            //...
            close(sock);
            return 0;
        }

        close(sock);
        return 0;
    }

    while (1)
    {
        cout << "Input message(Q to quit):" << endl;
        string msg_buff;
        getline(cin, msg_buff, '\n');

        copy(msg_buff.begin(), msg_buff.end(), message);
        message[msg_buff.size()] = 0;
        if (message == "q" || message == "Q")
            break;

        str_len = write(sock, message, strlen(message));
    }
    close(sock);
    return 0;
}