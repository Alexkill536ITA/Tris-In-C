#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

SOCKET mainSock;
char buffer[512];
char gameTable[3][3];
char name[30];
int retrn;

int receiveFrom(SOCKET *sock, char *buffer);
int sendTo(SOCKET *sock, char *data);
//void checkCoord(char src, char* dst);
int checkCoord(char *src, char *dst);
void printGameTable();

int main(int argc, char *argv[])
{
    unsigned long ipServer;
    int portServer;
    char userIn[512];

    system("cls");
    printf("___________________________________________________________________\n\n");
    printf("   .___________..______       __       _______.\n");
    printf("   |           ||   _  \\     |  |     /       |\n");
    printf("   `---|  |----`|  |_)  |    |  |    |   (----`\n");
    printf("       |  |     |      /     |  |     \\   \\    \n");
    printf("       |  |     |  |\\  \\----.|  | .----)   |   \n");
    printf("       |__|     | _| `._____||__| |_______/     By Alexkill536ITA\n\n");
    printf("___________________________________________________________________\n\n");
    do
    {
        printf("Enter Nikname: ");
        gets(name);
        printf("\n");
        printf("Gimme the IP of server: ");
        memset(userIn, 0x00, sizeof(userIn));
        gets(userIn);
        ipServer = inet_addr(userIn);
        char tmp[512];
        memcpy(tmp, userIn, sizeof(userIn));
        do
        {
            printf("Gimme the Port of server: ");
            memset(userIn, 0x00, sizeof(userIn));
            gets(userIn);
            portServer = atoi(userIn);
            if (portServer <= 0 || portServer >= 65536)
                printf("Port not valid! Retry\n");
        } while (portServer <= 0 || portServer >= 65536);
        printf("Server IP: %s\nServer Port: %d\nContinue? [Y/n] ", tmp, portServer);
        gets(userIn);
        if (userIn[0] == '\n' || userIn[0] == '\r')
            userIn[0] = 'Y';
    } while (userIn[1] == '\0' && userIn[0] != 'Y' && userIn[0] != 'y');

    /*
    printf("Enter Nikname: Player\n");
    name[0] = 'T';
    printf("Gimme the IP of server: 127.0.0.1\n");
    ipServer = inet_addr("127.0.0.1");
    printf("Gimme the Port of server: 5835\n");
    portServer = 5835;
    printf("Server IP: 127.0.0.1\nServer Port: 5835\nContinue? [Y/n] Y\n");
    */


    struct sockaddr_in config;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("[ ERROR ] Failed WSAStartup() with error: %d\n", WSAGetLastError());
        return 1;
    }
    printf("[  OK   ] WSAStartup Complete!\n");

    memset(&config, 0x00, sizeof(struct sockaddr_in));

    mainSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mainSock == INVALID_SOCKET)
    {
        printf("[ ERROR ] Failed main socket creation!\n");
        perror("socket");
        return 2;
    }
    printf("[  OK   ] Socket created!\n");

    config.sin_family = AF_INET;
    config.sin_addr.s_addr = ipServer;
    config.sin_port = htons(portServer);

    if ((retrn = connect(mainSock, (struct sockaddr *)&config, sizeof(struct sockaddr_in))) == SOCKET_ERROR)
    {
        printf("[ ERROR ] Failed Connect() with error: %d\n", WSAGetLastError());
    }
    else
    {
        printf("[  OK   ] Connected!\n");
        do
        {
            memset(buffer, '\0', sizeof(char) * 512);
            if ((retrn = receiveFrom(&mainSock, buffer)) == 0)
            {
                if (strcmp(buffer, "Start") == 0)
                {
                    char coords[] = {-1, -1};
                    int x, y, ext = 0;
                    printGameTable();
                    do
                    {
                        memset(userIn, 0x00, sizeof(userIn));
                        printf("%s Dammi le coordinate: ",name);
                        gets(userIn);
                        //checkCoord(userIn[0], &coords[0]);
                        //checkCoord(userIn[1], &coords[1]);

                        if (checkCoord(userIn, coords) != 0 || coords[0] == -1 || coords[1] == -1)
                        {
                            printf("Cordinate Errate\n");
                        }
                        /*
                        x = coords[0];
                        y = coords[1];
                        if (gameTable[x][y] == 'X' || gameTable[x][y] == 'O')
                        {
                            printf("Casella già occupata\n");
                            ext = 1;
                        }
                        else
                        {
                            ext = 0;
                        }
                        */

                    } while (coords[0] == -1 || coords[1] == -1);
                    if ((retrn = sendTo(&mainSock, coords)) != 0)
                    {
                        printf("[ ERROR ] Failed receiveFrom(&mainSock, buffer) with error: %d\n", retrn);
                        break;
                    }
                }
                else if (strcmp(buffer, "Wait") == 0)
                {
                    printGameTable();
                    printf("Attendi l'altro giocatore...\n");
                }
                else if (strcmp(buffer, "Win") == 0)
                {
                    printf("%s YOU WIN \n",name);
                    break;
                }
                else if (strcmp(buffer, "Lose") == 0)
                {
                    printf("%s YOU LOSE \n",name);
                    break;
                }
                else if (strcmp(buffer, "Draw") == 0)
                {
                    printf("%s GAME IS DRAW \n",name);
                    break;
                }
                else if (strcmp(buffer, "Exit") == 0)
                {
                    printf("Out for random choice\n");
                    break;
                }
                else
                {
                    memccpy(gameTable, buffer, 0, sizeof(gameTable));
                    printGameTable();
                }

            }
            else
            {
                printf("[ ERROR ] Failed receiveFrom(&mainSock, buffer) with error: %d\n", retrn);
                break;
            }
        } while (strcmp(buffer, "Win") != 0 && strcmp(buffer, "Lose") != 0 && strcmp(buffer, "Draw") != 0);
    }
    closesocket(mainSock);

    WSACleanup();
    printf("\n");
    system("pause");
    return 0;
}

int receiveFrom(SOCKET *sock, char *buffer)
{
    int countRecv = recv(*sock, buffer, 512, 0);
    if (countRecv > 0)
    {
        //printf("[DEBUG] Received: %s\n", buffer);
        return 0;
    }
    else if (countRecv == 0)
    {
        printf("[DEBUG] Connection closed\n");
        return -1;
    }
    else
    {
        int err = WSAGetLastError();
        printf("[DEBUG] recv failed: %d\n", err);
        return err;
    }
}
int sendTo(SOCKET *sock, char *data)
{
    if (send(*sock, data, strlen(data), 0x00) == SOCKET_ERROR)
        return 1;
    return 0;
}

void printGameTable()
{
    system("cls");
    printf("___________________________________________________________________\n\n");
    printf("   .___________..______       __       _______.\n");
    printf("   |           ||   _  \\     |  |     /       |\n");
    printf("   `---|  |----`|  |_)  |    |  |    |   (----`\n");
    printf("       |  |     |      /     |  |     \\   \\    \n");
    printf("       |  |     |  |\\  \\----.|  | .----)   |   \n");
    printf("       |__|     | _| `._____||__| |_______/     By Alexkill536ITA\n\n");
    printf("___________________________________________________________________\n\n");
    printf("                       /---------------\\\n");
    printf("                       |   | A | B | C |\n");
    printf("                       |---|---|---|---|\n");
    printf("                       | 1 | %c | %c | %c |\n", gameTable[0][0], gameTable[0][1], gameTable[0][2]);
    printf("                       |---|---|---|---|\n");
    printf("                       | 2 | %c | %c | %c |\n", gameTable[1][0], gameTable[1][1], gameTable[1][2]);
    printf("                       |---|---|---|---|\n");
    printf("                       | 3 | %c | %c | %c |\n", gameTable[2][0], gameTable[2][1], gameTable[2][2]);
    printf("                       |---|---|---|---|\n");
    printf("                       \\---------------/\n\n");
}

//void checkCoord(char src, char* dst) {
//    switch (src) {
//        case 'A':
//        case 'B':
//        case 'C':
//        case '1':
//        case '2':
//        case '3':
//            dst[0] = src;
//            break;
//    }
//}

int checkCoord(char *src, char *dst)
{
    int len = strlen(src);
    if (len != 2)
        return 1;
    for (int i = 0; i < len; i++)
    {
        switch (src[i])
        {
        case 'A':
        case 'B':
        case 'C':
            if (i > 0)
            {
                dst[i] = -1;
                return 1;
            }
            dst[i] = src[i];
            break;
        case '1':
        case '2':
        case '3':
            if (i < 1)
                return 1;
            dst[i] = src[i];
            break;
        }
    }
    return 0;
}
