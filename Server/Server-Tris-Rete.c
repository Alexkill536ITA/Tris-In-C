#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <time.h>

SOCKET mainSock, p1, p2;
char buffer[512];
char gameTable[3][3];
int dim = 0;
int list[255];
char IPserver[255][16];
int enable_seselect;
char select_ip;
SOCKET *waitSock;

//#define IPserver  "172.0.0.1"//"192.168.5.121"
int setupSocketMain(int port);
int receiveFrom(SOCKET *sock, char *buffer);
int sendTo(SOCKET *sock, char *data);
int WinnerCheck();
void checkCoord(char src, char *dst);
void printGameTable();
char *BinIP2StrIP(long lnIP);
void GetIpAddress();
void get_time();

int main(int argc, char *argv[])
{
    int retrn = 0;
    int listPort = -1;
    memset(&buffer, 0x00, 512);
    memset(&gameTable, ' ', sizeof(gameTable));
    char userIn[50];
    int round = 0;

    get_time();
    printf("[  INFO ] Start Server Manager\n");
    do
    {
        get_time();
        printf("[  GET  ] Gimme the port for wait the connections from clients: ");
        memset(userIn, 0x00, sizeof(userIn));
        gets(userIn);
        listPort = atoi(userIn);
        if (listPort <= 0 || listPort >= 65536)
        {
            get_time();
            printf("[WARNING] Port not valid! Retry\n");
        }
        else if (listPort <= 1024)
        {
            get_time();
            printf("[WARNING] Ports under 1024 aren't usable! Retry\n");
        }
    } while (listPort <= 1024 || listPort >= 65536);

    /*
    printf("[  GET  ] Gimme the port for wait the connections from clients: 5835\n");
    listPort = 5835;
    */

    /*
	GetIpAddress();
    for (size_t i = 0; i < dim; i++)
    {
        get_time();
        printf("[  INFO ] IP Addres %d: %s \n", i, IPserver[i]);
    }
	
    get_time();
    printf("[  GET  ] Select To IP Addres: ");
    gets(&select_ip);
    get_time();
    printf("[  INFO ] Local Machine IP Adrees: %s\n", IPserver[atoi(&select_ip)]);
	*/
	
    retrn = setupSocketMain(listPort);

    if (retrn == 0)
    {
        if ((p1 = accept(mainSock, NULL, NULL)) == SOCKET_ERROR)
        {
            get_time();
            printf("[ ERROR ] Failed Accept() with error: %d\n", WSAGetLastError());
            retrn = 1;
        }
        else
        {
            waitSock = &p1;
            if (sendTo(&p1, "Wait") != 0)
            {
                get_time();
                printf("[ ERROR ] Failed sendTo(&p1, \"Wait\") with error: %d\n", WSAGetLastError());
                retrn = 1;
            }
            else
            {
                get_time();
                printf("[  INFO ] P1 Connectd\n");
                if ((p2 = accept(mainSock, NULL, NULL)) == SOCKET_ERROR)
                {
                    get_time();
                    printf("[ ERROR ] Failed Accept() with error: %d\n", WSAGetLastError());
                    retrn = 1;
                }
                else
                {
                    get_time();
                    printf("[  INFO ] P2 Connectd\n");
                    do
                    {
                        if (waitSock == &p1)
                        {
                            if ((retrn = sendTo(&p2, "Wait")) != 0)
                            {
                                get_time();
                                printf("[ ERROR ] Failed sendTo(&p2, \"Wait\") with error: %d\n", retrn);
                                retrn = 1;
                                waitSock = 0;
                            }
                            else
                            {
                                waitSock = &p2;
                                if ((retrn = sendTo(&p1, "Start")) != 0)
                                {
                                    get_time();
                                    printf("[ ERROR ] Failed sendTo(&p1, \"Start\") with error: %d\n", retrn);
                                    retrn = 1;
                                    waitSock = 0;
                                }
                                else
                                {
                                    memset(&buffer, 0x00, 512);
                                    if ((retrn = receiveFrom(&p1, buffer)) != 0)
                                    {
                                        get_time();
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
                                            get_time();
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
                                                get_time();
                                                printf("[ ERROR ] Failed sendTo(&p1, gameTable) with error: %d\n", retrn);
                                                retrn = 1;
                                                waitSock = 0;
                                            }
                                            if ((retrn = sendTo(&p2, tmpTable)) != 0)
                                            {
                                                get_time();
                                                printf("[ ERROR ] Failed sendTo(&p2, gameTable) with error: %d\n", retrn);
                                                retrn = 1;
                                                waitSock = 0;
                                            }

                                            Wincheck = WinnerCheck();
                                            if (Wincheck == 1)
                                            {
                                                if ((retrn = sendTo(&p1, "Win")) != 0)
                                                {
                                                    get_time();
                                                    printf("[ ERROR ] Failed sendTo(&p1, 'Win') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                if ((retrn = sendTo(&p2, "Lose")) != 0)
                                                {
                                                    get_time();
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
                                                    get_time();
                                                    printf("[ ERROR ] Failed sendTo(&p1, 'Win') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                if ((retrn = sendTo(&p1, "Lose")) != 0)
                                                {
                                                    get_time();
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
                                get_time();
                                printf("[ ERROR ] Failed sendTo(&p1, \"Wait\") with error: %d\n", retrn);
                                retrn = 1;
                                waitSock = 0;
                            }
                            else
                            {
                                waitSock = &p1;
                                if ((retrn = sendTo(&p2, "Start")) != 0)
                                {
                                    get_time();
                                    printf("[ ERROR ] Failed sendTo(&p2, \"Start\") with error: %d\n", retrn);
                                    retrn = 1;
                                    waitSock = 0;
                                }
                                else
                                {
                                    memset(&buffer, 0x00, 512);
                                    if ((retrn = receiveFrom(&p2, buffer)) != 0)
                                    {
                                        get_time();
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
                                            get_time();
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
                                                get_time();
                                                printf("[ ERROR ] Failed sendTo(&p1, gameTable) with error: %d\n", retrn);
                                                retrn = 1;
                                                waitSock = 0;
                                            }
                                            if ((retrn = sendTo(&p2, tmpTable)) != 0)
                                            {
                                                get_time();
                                                printf("[ ERROR ] Failed sendTo(&p2, gameTable) with error: %d\n", retrn);
                                                retrn = 1;
                                                waitSock = 0;
                                            }

                                            Wincheck = WinnerCheck();
                                            if (Wincheck == 1)
                                            {
                                                if ((retrn = sendTo(&p1, "Win")) != 0)
                                                {
                                                    get_time();
                                                    printf("[ ERROR ] Failed sendTo(&p1, 'Win') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                if ((retrn = sendTo(&p2, "Lose")) != 0)
                                                {
                                                    get_time();
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
                                                    get_time();
                                                    printf("[ ERROR ] Failed sendTo(&p1, 'Win') with error: %d\n", retrn);
                                                    retrn = 1;
                                                    waitSock = 0;
                                                }
                                                if ((retrn = sendTo(&p1, "Lose")) != 0)
                                                {
                                                    get_time();
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
                            get_time();
                            printf("[ ERROR ] Failed sendTo(&p1, 'Draw') with error: %d\n", retrn);
                            retrn = 1;
                            waitSock = 0;
                        }
                        if ((retrn = sendTo(&p1, "Draw")) != 0)
                        {
                            get_time();
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
    get_time();
    printf("[  INFO ] Stop Server Manager\n\n");
    system("pause");
    return retrn;
}

char *BinIP2StrIP(long lnIP)
{
    static char s_acStrIP[16];

    PUCHAR puc = (unsigned char *)&lnIP;
    unsigned int aun[4];

    for (int i = 0; i < 4; aun[i++] = *puc++)
        ;

    sprintf(s_acStrIP, "%d.%d.%d.%d", aun[0], aun[1], aun[2], aun[3]);
    sprintf(IPserver[dim], "%d.%d.%d.%d", aun[0], aun[1], aun[2], aun[3]);

    return (s_acStrIP);
}

void GetIpAddress()
{

    char acClientHost[255];
    WORD wVersionRequested;
    WSADATA wsaData;
    struct hostent *pHostent = NULL;
    int nErr = 0;

    wVersionRequested = MAKEWORD(1, 1);

    if (nErr = WSAStartup(wVersionRequested, &wsaData))
    {
        // error initializing winsck...
    }

    if (nErr = gethostname((char *)acClientHost, sizeof(acClientHost)))
    {
        //return NULL;
    }

    if (!(pHostent = gethostbyname(acClientHost)))
    {
        // an error occured...
        //return NULL;
    }

    for (size_t i = 0; i < pHostent->h_length; i++)
    {
        if (pHostent->h_length == 1)
        {
            select_ip = '0';
            enable_seselect = 0;
        }
        list[i] = inet_addr(BinIP2StrIP(*((long *)pHostent->h_addr_list[i])));
        printf("%d \n", list[i]);
        dim++;
    }
}

int setupSocketMain(int port)
{
    struct sockaddr_in config;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        get_time();
        printf("[ ERROR ] Failed WSAStartup() with error: %d", WSAGetLastError());
        return 1;
    }
    get_time();
    printf("[  Ok   ] WSAStartup Complete!\n");

    memset(&config, 0x00, sizeof(struct sockaddr_in));

    mainSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mainSock == INVALID_SOCKET)
    {
        get_time();
        printf("[ ERROR ] Failed main socket creation!\n");
        perror("socket");
        return 2;
    }
    get_time();
    printf("[  OK   ] Socket created!\n");

    config.sin_family = AF_INET;
    /*
	if (enable_seselect == 1)
    {
        config.sin_addr.s_addr = list[atoi(&select_ip)];
    }
    else
    {
        config.sin_addr.s_addr = list[0];
    }
	*/
	config.sin_addr.s_addr = INADDR_ANY;
    config.sin_port = htons(port);

    if (bind(mainSock, (struct sockaddr *)&config, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
    {
        get_time();
        printf("[ ERROR ] Failed main socket binding! %d\n", WSAGetLastError());
        return 2;
    }
    get_time();
    printf("[  OK   ] Bind Success!\n");

    if (listen(mainSock, 10) == SOCKET_ERROR)
    {
        get_time();
        printf("[ERROR] Failed to open listen port!\n");
        perror("listen");
        return 2;
    }
    get_time();
    printf("[  INFO ] Listening Port Open!\n");

    return 0;
}

int receiveFrom(SOCKET *sock, char *buffer)
{
    int countRecv = recv(*sock, buffer, 512, 0);
    if (countRecv > 0)
    {
        get_time();
        printf("[ DEBUG ] Received: %s \n", buffer);
        return 0;
    }
    else if (countRecv == 0)
    {
        get_time();
        printf("[ DEBUG ] Connection closed\n");
        return -1;
    }
    else
    {
        int err = WSAGetLastError();
        get_time();
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

void get_time()
{
    char s[100];
    int dim;
    time_t t = time(NULL);
    struct tm *tp = localtime(&t);

    dim = strftime(s, 100, "%H:%M:%S", tp);
    printf("%s ", s);
}