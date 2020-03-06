#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>

SOCKET mainSock, p1, p2;
char buffer[512];
char gameTable[3][3];
SOCKET *waitSock;

int setupSocketMain(int port);
int receiveFrom(SOCKET *sock, char *buffer);
int sendTo(SOCKET *sock, char *data);
int WinnerCheck();
void checkCoord(char src, char *dst);
void printGameTable();

int main(int argc, char *argv[])
{
    int retrn = 0;
    int listPort = -1;
    memset(&buffer, 0x00, 512);
    memset(&gameTable, ' ', sizeof(gameTable));
    char userIn[50];
    int round = 0;

    printf("[  INFO ] Start Server Manager\n");
    do
    {
        printf("[  GET  ] Gimme the port for wait the connections from clients: ");
        memset(userIn, 0x00, sizeof(userIn));
        gets(userIn);
        listPort = atoi(userIn);
        if (listPort <= 0 || listPort >= 65536)
            printf("[WARNING] Port not valid! Retry\n");
        else if (listPort <= 1024)
            printf("[WARNING] Ports under 1024 aren't usable! Retry\n");
    } while (listPort <= 1024 || listPort >= 65536);

    /*
    printf("[  GET  ] Gimme the port for wait the connections from clients: 5835\n");
    listPort = 5835;
    */

    retrn = setupSocketMain(listPort);

    if (retrn == 0)
    {
        if ((p1 = accept(mainSock, NULL, NULL)) == SOCKET_ERROR)
        {
            printf("[ ERROR ] Failed Accept() with error: %d\n", WSAGetLastError());
            retrn = 1;
        }
        else
        {
            waitSock = &p1;
            if (sendTo(&p1, "Wait") != 0)
            {
                printf("[ ERROR ] Failed sendTo(&p1, \"Wait\") with error: %d\n", WSAGetLastError());
                retrn = 1;
            }
            else
            {
                printf("[  INFO ] P1 Connectd\n");
                if ((p2 = accept(mainSock, NULL, NULL)) == SOCKET_ERROR)
                {
                    printf("[ ERROR ] Failed Accept() with error: %d\n", WSAGetLastError());
                    retrn = 1;
                }
                else
                {
                    printf("[  INFO ] P2 Connectd\n");
                    do
                    {
                        if (waitSock == &p1)
                        {
                            if ((retrn = sendTo(&p2, "Wait")) != 0)
                            {
                                printf("[ ERROR ] Failed sendTo(&p2, \"Wait\") with error: %d\n", retrn);
                                retrn = 1;
                                waitSock = 0;
                            }
                            else
                            {
                                waitSock = &p2;
                                if ((retrn = sendTo(&p1, "Start")) != 0)
                                {
                                    printf("[ ERROR ] Failed sendTo(&p1, \"Start\") with error: %d\n", retrn);
                                    retrn = 1;
                                    waitSock = 0;
                                }
                                else
                                {
                                    memset(&buffer, 0x00, 512);
                                    if ((retrn = receiveFrom(&p1, buffer)) != 0)
                                    {
                                        printf("[ ERROR ] Failed receiveFrom(&p1, buffer) with error: %d\n", retrn);
                                        retrn = 1;
                                        waitSock = 0;
                                    }
                                    else
                                    {
                                        char coords[2];
                                        memset(&coords, 0, sizeof(coords));
                                        checkCoord(buffer[0], coords);
                                        checkCoord(buffer[1], coords);
                                        if (retrn != 0)
                                        {
                                            printf("[WARNING] Received Wrong Coords: %s", buffer);
                                            retrn = 1;
                                            waitSock = 0;
                                        }
                                        else
                                        {
                                            int x = coords[0], y = coords[1];
                                            gameTable[y][x] = 'X';
                                            char tmpTable[9];
                                            int Wincheck = 0;
                                            memccpy(tmpTable, gameTable, 0, sizeof(gameTable));
                                            if ((retrn = sendTo(&p1, tmpTable)) != 0)
                                            {
                                                printf("[ ERROR ] Failed sendTo(&p1, gameTable) with error: %d\n", retrn);
                                                retrn = 1;
                                                waitSock = 0;
                                            }
                                            if ((retrn = sendTo(&p2, tmpTable)) != 0)
                                            {
                                                printf("[ ERROR ] Failed sendTo(&p2, gameTable) with error: %d\n", retrn);
                                                retrn = 1;
                                                waitSock = 0;
                                            }

                                            Wincheck = WinnerCheck();
                                            if (Wincheck == 1)
                                            {
                                                if ((retrn = sendTo(&p1, "Win")) != 0)
                                                {
                                                    printf("[ ERROR ] Failed sendTo(&p1, 'Win') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                if ((retrn = sendTo(&p2, "Lose")) != 0)
                                                {
                                                    printf("[ ERROR ] Failed sendTo(&p2, 'Lose') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                break;
                                            }
                                            else if (Wincheck == 2)
                                            {
                                                if ((retrn = sendTo(&p2, "Win")) != 0)
                                                {
                                                    printf("[ ERROR ] Failed sendTo(&p1, 'Win') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                if ((retrn = sendTo(&p1, "Lose")) != 0)
                                                {
                                                    printf("[ ERROR ] Failed sendTo(&p2, 'Lose') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                break;
                                            }
                                            round++;
                                            if (round == 9)
                                            {
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (waitSock == &p2)
                        {
                            if ((retrn = sendTo(&p1, "Wait")) != 0)
                            {
                                printf("[ ERROR ] Failed sendTo(&p1, \"Wait\") with error: %d\n", retrn);
                                retrn = 1;
                                waitSock = 0;
                            }
                            else
                            {
                                waitSock = &p1;
                                if ((retrn = sendTo(&p2, "Start")) != 0)
                                {
                                    printf("[ ERROR ] Failed sendTo(&p2, \"Start\") with error: %d\n", retrn);
                                    retrn = 1;
                                    waitSock = 0;
                                }
                                else
                                {
                                    memset(&buffer, 0x00, 512);
                                    if ((retrn = receiveFrom(&p2, buffer)) != 0)
                                    {
                                        printf("[ ERROR ] Failed receiveFrom(&p2, buffer) with error: %d\n", retrn);
                                        retrn = 1;
                                        waitSock = 0;
                                    }
                                    else
                                    {
                                        char coords[2];
                                        memset(&coords, 0, sizeof(coords));
                                        checkCoord(buffer[0], coords);
                                        checkCoord(buffer[1], coords);
                                        if (retrn != 0)
                                        {
                                            printf("[WARNING] Received Wrong Coords: %s", buffer);
                                            retrn = 1;
                                            waitSock = 0;
                                        }
                                        else
                                        {
                                            int x = coords[0], y = coords[1];
                                            gameTable[y][x] = 'O';
                                            char tmpTable[9];
                                            int Wincheck = 0;
                                            memccpy(tmpTable, gameTable, 0, sizeof(gameTable));
                                            if ((retrn = sendTo(&p1, tmpTable)) != 0)
                                            {
                                                printf("[ ERROR ] Failed sendTo(&p1, gameTable) with error: %d\n", retrn);
                                                retrn = 1;
                                                waitSock = 0;
                                            }
                                            if ((retrn = sendTo(&p2, tmpTable)) != 0)
                                            {
                                                printf("[ ERROR ] Failed sendTo(&p2, gameTable) with error: %d\n", retrn);
                                                retrn = 1;
                                                waitSock = 0;
                                            }

                                            Wincheck = WinnerCheck();
                                            if (Wincheck == 1)
                                            {
                                                if ((retrn = sendTo(&p1, "Win")) != 0)
                                                {
                                                    printf("[ ERROR ] Failed sendTo(&p1, 'Win') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                if ((retrn = sendTo(&p2, "Lose")) != 0)
                                                {
                                                    printf("[ ERROR ] Failed sendTo(&p2, 'Lose') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                break;
                                            }
                                            else if (Wincheck == 2)
                                            {
                                                if ((retrn = sendTo(&p2, "Win")) != 0)
                                                {
                                                    printf("[ ERROR ] Failed sendTo(&p1, 'Win') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                if ((retrn = sendTo(&p1, "Lose")) != 0)
                                                {
                                                    printf("[ ERROR ] Failed sendTo(&p2, 'Lose') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                break;
                                            }
                                            round++;
                                            if (round == 9)
                                            {
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                            break;
                    } while (waitSock != 0);
                    if (round == 9)
                    {
                        if ((retrn = sendTo(&p2, "Draw")) != 0)
                        {
                            printf("[ ERROR ] Failed sendTo(&p1, 'Draw') with error: %d\n", retrn);
                            retrn = 1;
                            waitSock = 0;
                        }
                        if ((retrn = sendTo(&p1, "Draw")) != 0)
                        {
                            printf("[ ERROR ] Failed sendTo(&p2, 'Draw') with error: %d\n", retrn);
                            retrn = 1;
                            waitSock = 0;
                        }
                    }
                }
            }
        }
    }
    closesocket(mainSock);
    closesocket(p1);
    closesocket(p2);
    WSACleanup();
    printf("[  INFO ] Stop Server Manager\n");
    system("pause");
    return retrn;
}

int setupSocketMain(int port)
{
    struct sockaddr_in config;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("[ ERROR ] Failed WSAStartup() with error: %d", WSAGetLastError());
        return 1;
    }
    printf("[  Ok   ] WSAStartup Complete!\n");

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
    config.sin_addr.s_addr = INADDR_ANY;
    config.sin_port = htons(port);

    if (bind(mainSock, (struct sockaddr *)&config, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
    {
        printf("[ ERROR ] Failed main socket binding! %d\n", WSAGetLastError());
        return 2;
    }
    printf("[  OK   ] Bind Success!\n");

    if (listen(mainSock, 10) == SOCKET_ERROR)
    {
        printf("[ERROR] Failed to open listen port!\n");
        perror("listen");
        return 2;
    }
    printf("[  INFO ] Listening Port Open!\n");

    return 0;
}

int receiveFrom(SOCKET *sock, char *buffer)
{
    int countRecv = recv(*sock, buffer, 512, 0);
    if (countRecv > 0)
    {
        printf("[ DEBUG ] Received: %s \n", buffer);
        return 0;
    }
    else if (countRecv == 0)
    {
        printf("[ DEBUG ] Connection closed\n");
        return -1;
    }
    else
    {
        int err = WSAGetLastError();
        printf("[ DEBUG ] recv failed: %d\n", err);
        return err;
    }
}
int sendTo(SOCKET *sock, char *data)
{
    if (send(*sock, data, strlen(data), 0x00) == SOCKET_ERROR)
        return 1;
    return 0;
}

void checkCoord(char src, char *dst)
{
    switch (src)
    {
    case 'A':
        dst[0] = 0;
        break;
    case 'B':
        dst[0] = 1;
        break;
    case 'C':
        dst[0] = 2;
        break;
    case '1':
    case '2':
    case '3':
        dst[1] = (char)src - '1';
        break;
    }
}

int WinnerCheck()
{
    int c;
    for (c = 0; c < 3; c++) // controllo se ci sono tris sulle righe
    {
        if (gameTable[c][0] + gameTable[c][1] - 2 * (gameTable[c][2]) == 0) //se c'è tris la somma delle prime 2 celle meno il doppio dell'ultima mi deve dare0
        {
            if (gameTable[c][0] == 'X')
                return 1;
            if (gameTable[c][0] == 'O')
                return 2;
        }
    }
    for (c = 0; c < 3; c++) // controllo se ci sono tris sulle colonne
    {
        if (gameTable[0][c] + gameTable[1][c] - 2 * (gameTable[2][c]) == 0)
        {
            if (gameTable[0][c] == 'X') //se questo if è vero vuol dire che il giocatore 1 ha vinto
                return 1;
            if (gameTable[0][c] == 'O')
                return 2;
        }
    }

    if (gameTable[0][0] + gameTable[1][1] - 2 * (gameTable[2][2]) == 0) //controllo diagonale 1
    {
        if (gameTable[1][1] == 'X') //se questo if è vero vuol dire che il giocatore 1 ha vinto
            return 1;
        if (gameTable[1][1] == 'O')
            return 2;
    }
    if (gameTable[0][2] + gameTable[1][1] - 2 * (gameTable[2][0]) == 0) //controllo diagonale 2
    {
        if (gameTable[0][2] == 'X') //se questo if è vero vuol dire che il giocatore 1 ha vinto
            return 1;
        if (gameTable[0][2] == 'O')
            return 2;
    }

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
