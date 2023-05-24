#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 5555
#define MAX_MSG 100

#define ROW_TABLE 6
#define COL_TABLE 7

void show_table(int table[ROW_TABLE][COL_TABLE], int player) {
    printf("          Connect 4        \n");
    printf("  1   2   3   4   5   6   7\n");
    for(int i = 0; i < 6; i++) {
        for(int j = 0; j < 7; j++) {
            printf("|_%c_", table[i][j]== 1 ? 'X' : table[i][j] == 2 ? 'O' : '_');
        }
        printf("|\n");
    }
    printf("\n");
    printf("You are %c\n", player == 1 ? 'X' : 'O');
}

int main(int argc, char *argv[])
{

    int sd, rc, i;
    struct sockaddr_in localAddr, servAddr;
    struct hostent *h;

    if (argc < 2)
    {
        printf("usage: %s <server> <data1> <data2> ... <dataN>\n", argv[0]);
        exit(1);
    }

    h = gethostbyname(argv[1]);
    if (h == NULL)
    {
        printf("%s: unknown host '%s'\n", argv[0], argv[1]);
        exit(1);
    }

    servAddr.sin_family = h->h_addrtype;
    memcpy((char *)&servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    servAddr.sin_port = htons(SERVER_PORT);

    /* create socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("cannot open socket ");
        exit(1);
    }

    /* bind any port number */
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    rc = bind(sd, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (rc < 0)
    {
        printf("%s: cannot bind port TCP %u\n", argv[0], SERVER_PORT);
        perror("error ");
        exit(1);
    }

    /* connect to server */
    rc = connect(sd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (rc < 0)
    {
        perror("cannot connect ");
        exit(1);
    }

    char recvBuff[1024];
    
    memset(recvBuff, '0',sizeof(recvBuff));
    
    int n, state = 0;
    while ((n = read(sd, recvBuff, sizeof(recvBuff) - 1)) > 0)
    {
        printf("\e[1;1H\e[2J");
        recvBuff[n] = 0;
        if(state) {
            int table[6][7] = {0};
            char* data = strtok(recvBuff, ",");
            int turn = -1, i, j = -1, wrong_input = -1, winner = 2, cwin = -1, player = -1;
            while(data != NULL) {
                if(player < 0) {
                    player = atoi(data);
                    data = strtok(NULL, ",");
                    continue;
                }
                if(turn < 0) {
                    turn = atoi(data);
                    j = 0;
                    data = strtok(NULL, ",");
                    continue;
                }
                if(cwin < 0){
                    winner = atoi(data);
                    data = strtok(NULL, ",");
                    cwin = 1;
                    continue;
                }
                if(wrong_input < 0) {
                    wrong_input = atoi(data);
                    data = strtok(NULL, ",");
                    continue;
                }
                if(j >= 0) {
                    int col = atoi(data);
                    for(i = 1; i < 7; i++) {
                        
                        table[i-1][j] = col % 10;
                        col /= 10;
                    }
                    j++;
                }
                data = strtok(NULL, ",");
            }
            show_table(table, player);
            
            if(winner != 2) {
                if(winner == 1) printf("You win!\n");
                else if(winner == 0) printf("You lose!\n");
                else if(winner == 3) printf("Draw!\n");
                if(player == 1) {
                    char sendBuff[20];
                    memset(sendBuff, '0', sizeof(sendBuff));
                    printf("Play again? (y/n): ");
                    scanf("%s", sendBuff);
                    rc = send(sd, sendBuff, strlen(sendBuff) + 1, 0);
                    if (rc < 0)
                    {
                        perror("cannot send data ");
                        close(sd);
                        exit(1);
                    }
                }
                state = 0;
            }
            else if(turn == 1) {
                if(wrong_input == 1)
                {
                    printf("Wrong input, try again\n");
                }
                char sendBuff[20];
                memset(sendBuff, '0', sizeof(sendBuff));
                printf("Enter column: ");
                scanf("%s", sendBuff);
                rc = send(sd, sendBuff, strlen(sendBuff) + 1, 0);
                if (rc < 0)
                {
                    perror("cannot send data ");
                    close(sd);
                    exit(1);
                }
            } 
            else {
                printf("Opposite turn\n");
            }
        }
        else {
            if(strcmp(recvBuff, "Ready\n") == 0) {
                state = 1;
            }
            if (fputs(recvBuff, stdout) == EOF)
            {
                printf("\n Error : Fputs error\n");
            }
        }   
    }

    if (n < 0)
    {
        printf("\n Read error \n");
    }

    return 0;

}
