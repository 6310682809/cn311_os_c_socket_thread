#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>

#define SUCCESS 0
#define ERROR 1

#define END_LINE 0x0
#define SERVER_PORT 5555
#define MAX_MSG 100

#define ROW_TABLE 6
#define COL_TABLE 7

#define NTHREADS_CHECK_WIN 4

pthread_mutex_t winner_mutex;
int winner = 0;
int *ptr_winner = &winner;
int itable[6][7] = {0};
/* function */
int read_line();
void *check_win_bottom();
void *check_win_left_right();
void *check_win_topRight_bottomLeft();
void *check_win_topLeft_bottomRight();
void *check_win_topLeft_bottomRight();
void check_win();

int main(int argc, char *argv[])
{
    char sendBuff[1025];
    int sd, newSd1, newSd2, cliLen1, cliLen2;
    memset(sendBuff, '0', sizeof(sendBuff));
    struct sockaddr_in cliAddr1, cliAddr2, servAddr;
    char line1[MAX_MSG];
    char line2[MAX_MSG];

    /* create socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("cannot open socket ");
        return ERROR;
    }

    /* bind server port */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(SERVER_PORT);

    if (bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("cannot bind port ");
        return ERROR;
    }

    listen(sd, 5);

    while (1)
    {

        printf("%s: waiting for connection on port TCP %u\n", argv[0], SERVER_PORT);

        cliLen1 = sizeof(cliAddr1);

        // client1
        newSd1 = accept(sd, (struct sockaddr *)&cliAddr1, &cliLen1);

        snprintf(sendBuff, sizeof(sendBuff), "Connent 1/2, wait player2\n");
        write(newSd1, sendBuff, strlen(sendBuff));

        if (newSd1 < 0)
        {
            perror("cannot accept connection newSd");
            return ERROR;
        }

        // client2
        cliLen2 = sizeof(cliAddr2);

        newSd2 = accept(sd, (struct sockaddr *)&cliAddr2, &cliLen2);

        snprintf(sendBuff, sizeof(sendBuff), "Connent 2/2, Ready in 3 sec\n");
        write(newSd1, sendBuff, strlen(sendBuff));
        snprintf(sendBuff, sizeof(sendBuff), "Connent 2/2, Ready in 3 sec\n");
        write(newSd2, sendBuff, strlen(sendBuff));
        sleep(3);

        if (newSd2 < 0)
        {
            perror("cannot accept connection newSd");
            return ERROR;
        }

        if (newSd1 > 0 && newSd2 > 0)
        {
            char status[1024];
            memset(status, 0x0, 100);
            snprintf(status, sizeof(status), "Ready\n");
            write(newSd1, status, strlen(status));
            write(newSd2, status, strlen(status));
        }
        sleep(1);

        /* init line */
        memset(line1, 0x0, MAX_MSG);
        memset(line2, 0x0, MAX_MSG);

        // reset itable to 0 all
        memset(itable, 0, sizeof(itable));

        int table[7] = {1000000, 1000000, 1000000, 1000000,
                        1000000, 1000000, 1000000};
        char data[64];
        int turn = 1, *ptr_winner = 0;
        memset(data, 0x0, sizeof(data));
        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 1, turn, -(winner - 1) + 1, 0,
                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
        write(newSd1, data, strlen(data));

        memset(data, 0x0, sizeof(data));
        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 2, turn + 1, (winner + 2) % 3, 0,
                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
        write(newSd2, data, strlen(data));

        /* receive segments */
        int draw = 0;
        winner = 0;
        while (1)
        {
            /* init line */
            memset(line1, 0x0, MAX_MSG);
            memset(line2, 0x0, MAX_MSG);
            int n, i, j, over;
            if (turn == 1)
            {
                if (n = (read_line(newSd1, line1)) != ERROR)
                {
                    int col = atoi(line1), row;
                    col--;
                    if((winner != 0 && strcmp(line1, "y") == 0) || (winner == 0 && draw == 7 && strcmp(line1, "y") == 0)) {
                        winner = 0;
                        draw = 0;
                        char status[1024];
                        memset(status, 0x0, 100);
                        snprintf(status, sizeof(status), "Ready\n");
                        write(newSd1, status, strlen(status));
                        write(newSd2, status, strlen(status));

                        sleep(1);
                        memset(itable, 0, sizeof(itable));

                        for(int i = 0; i < 7; i++) {
                            table[i] = 1000000;
                        }

                        memset(data, 0x0, sizeof(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 1, turn, -(winner - 1) + 1, 0,
                                table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd1, data, strlen(data));

                        memset(data, 0x0, sizeof(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 2, turn + 1, (winner + 2) % 3, 0,
                                table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd2, data, strlen(data));

                        continue;
                    }
                    else if(winner != 0 && strcmp(line1, "y") != 0) {
                        winner = 0;
                        draw = 0;
                        close(newSd1);
                        close(newSd2);
                        break;
                    }
                    if (col < 0 || col > 6)
                    {
                        snprintf(sendBuff, sizeof(sendBuff), "%d,1,2,1,%d,%d,%d,%d,%d,%d,%d", 1,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd1, sendBuff, strlen(sendBuff));
                        continue;
                    }
                    for (i = 6; i >= 1; i--)
                    {
                        if (table[col] % (int)pow(10, i) == 0)
                        {
                            table[col] += (int)pow(10, i - 1);
                            row = i - 1;
                            itable[row][col] = turn;
                            if(row == 0) {
                                draw++;
                            }
                            break;
                        }
                    }
                    over = i;

                    check_win(row, col, turn);
                    if(draw == 7 && winner == 0) {
                        memset(data, 0x0, sizeof(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 1, 0, 3, 0,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd1, data, strlen(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 2, 1, 3, 0,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd2, data, strlen(data));
                        turn = 1;
                        continue;
                    } 

                    if (over == 0)
                    {
                        snprintf(sendBuff, sizeof(sendBuff), "%d,1,2,1,%d,%d,%d,%d,%d,%d,%d", 1,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd1, sendBuff, strlen(sendBuff));
                    }
                    else
                    {
                        memset(data, 0x0, sizeof(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 1, 0, -(winner - 1) + 1, 0,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd1, data, strlen(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 2, 1, (winner + 2) % 3, 0,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd2, data, strlen(data));
                        turn = 2;
                        if(winner != 0) {
                            turn = 1;
                        }
                        sleep(0.5);
                    }
                }
                else
                {
                    sleep(0.5);
                    break;
                }
            }
            else
            {
                if (n = (read_line(newSd2, line2)) != ERROR)
                {
                    int col = atoi(line2), row;
                    col--;
                    if (col < 0 || col > 6)
                    {
                        snprintf(sendBuff, sizeof(sendBuff), "%d,1,2,1,%d,%d,%d,%d,%d,%d,%d", 2,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd2, sendBuff, strlen(sendBuff));
                        continue;
                    }
                    for (i = 6; i >= 1; i--)
                    {
                        if (table[col] % (int)pow(10, i) == 0)
                        {
                            table[col] += (int)pow(10, i - 1) * 2;
                            row = i - 1;
                            itable[row][col] = turn;
                            if(row == 0) {
                                draw++;
                            }
                            break;
                        }
                    }
                    over = i;

                    check_win(row, col, turn);
                    if(draw == 7 && winner == 0) {
                        memset(data, 0x0, sizeof(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 1, 0, 3, 0,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd1, data, strlen(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 2, 1, 3, 0,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd2, data, strlen(data));
                        continue;
                    } 

                    if (over == 0)
                    {
                        snprintf(sendBuff, sizeof(sendBuff), "%d,1,2,1,%d,%d,%d,%d,%d,%d,%d", 2,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd2, sendBuff, strlen(sendBuff));
                    }
                    else
                    {
                        memset(data, 0x0, sizeof(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 1, 1, -(winner - 1) + 1, 0,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd1, data, strlen(data));
                        snprintf(data, sizeof(data), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 2, 0, (winner + 2) % 3, 0,
                                 table[0], table[1], table[2], table[3], table[4], table[5], table[6]);
                        write(newSd2, data, strlen(data));
                        turn = 1;
                        sleep(0.5);
                    }
                }
                else
                {
                    // close(newSd1);
                    // close(newSd2);
                    sleep(0.5);
                    break;
                    ;
                }
            }

        } /* while(read_line) */

    } /* while (1) */
    return 0;
}

int read_line(int newSd, char *line_to_return)
{

    static int rcv_ptr = 0;
    static char rcv_msg[MAX_MSG];
    static int n;
    int offset;

    offset = 0;

    while (1)
    {
        if (rcv_ptr == 0)
        {
            /* read data from socket */
            memset(rcv_msg, 0x0, MAX_MSG); /* init buffer */
            n = recv(newSd, rcv_msg, MAX_MSG, 0); /* wait for data */
            if (n < 0)
            {
                perror(" cannot receive data ");
                return ERROR;
            }
            else if (n == 0)
            {
                printf(" connection closed by client\n");
                close(newSd);
                return ERROR;
            }
        }

        /* if new data read on socket */
        /* OR */
        /* if another line is still in buffer */

        /* copy line into 'line_to_return' */
        while (*(rcv_msg + rcv_ptr) != END_LINE && rcv_ptr < n)
        {
            memcpy(line_to_return + offset, rcv_msg + rcv_ptr, 1);
            offset++;
            rcv_ptr++;
        }

        /* end of line + end of buffer => return line */
        if (rcv_ptr == n - 1)
        {
            /* set last byte to END_LINE */
            *(line_to_return + offset) = END_LINE;
            rcv_ptr = 0;
            return ++offset;
        }

        /* end of line but still some data in buffer => return line */
        if (rcv_ptr < n - 1)
        {
            /* set last byte to END_LINE */
            *(line_to_return + offset) = END_LINE;
            rcv_ptr++;
            return ++offset;
        }

        /* end of buffer but line is not ended => */
        /*  wait for more data to arrive on socket */
        if (rcv_ptr == n)
        {
            rcv_ptr = 0;
        }

    } /* while */
}

void *check_win_bottom(void *arg)
{
    int *params = (int *)arg;
    int tid = params[0];
    int row = params[1];
    int col = params[2];
    int turn = params[3];

    int count = 1;
    for (int i = row; i < 6; i++)
    {
        if (i + 1 != 6)
        {
            if (itable[i + 1][col] == turn)
                count++;
            else
                break;
        }
        else
            break;
    }
    pthread_mutex_lock(&winner_mutex);
    if (*ptr_winner == 0)
    {
        *ptr_winner = count >= 4 ? turn : 0;
        if(*ptr_winner >= 4) printf("check_win_bottom\n");
    }
    pthread_mutex_unlock(&winner_mutex);
    pthread_exit(NULL);
}

void *check_win_left_right(void *arg)
{
    int *params = (int *)arg;
    int tid = params[0];
    int row = params[1];
    int col = params[2];
    int turn = params[3];

    int count = 1;
    for (int i = col; i > 0; i--)
    {
        if (i - 1 != -1)
        {
            if (itable[row][i - 1] == turn)
                count++;
            else
                break;
        }
        else
            break;
    }
    for (int i = col; i < 7; i++)
    {
        if (i + 1 != 7)
        {
            if (itable[row][i + 1] == turn)
                count++;
            else
                break;
        }
        else
            break;
    }
    pthread_mutex_lock(&winner_mutex);
    if (*ptr_winner == 0)
    {
        *ptr_winner = count >= 4 ? turn : 0;
        if(*ptr_winner >= 4) printf("check_win_left_right\n");
    }
    pthread_mutex_unlock(&winner_mutex);
    pthread_exit(NULL);
}

void *check_win_topRight_bottomLeft(void *arg)
{
    int *params = (int *)arg;
    int tid = params[0];
    int row = params[1];
    int col = params[2];
    int turn = params[3];

    int count = 1;
    for (int i = row, j = col; i > 0, j < 7; i--, j++)
    {
        if (i - 1 != -1 || j + 1 != 7)
        {
            if (itable[i - 1][j + 1] == turn)
                count++;
            else
                break;
        }
        else
            break;
    }
    for (int i = row, j = col; i<6, j> 0; i++, j--)
    {
        if (i + 1 != 6 || j - 1 != -1)
        {
            if (itable[i + 1][j - 1] == turn)
                count++;
            else
                break;
        }
        else
            break;
    }
    pthread_mutex_lock(&winner_mutex);
    if (*ptr_winner == 0)
    {
        *ptr_winner = count >= 4 ? turn : 0;
        if(*ptr_winner >= 4) printf("check_win_topRight_bottomLeft\n");
    }
    pthread_mutex_unlock(&winner_mutex);
    pthread_exit(NULL);
}

void *check_win_topLeft_bottomRight(void *arg)
{
    int *params = (int *)arg;
    int tid = params[0];
    int row = params[1];
    int col = params[2];
    int turn = params[3];

    int count = 1;
    for (int i = row, j = col; i > 0, j > 0; i--, j--)
    {
        if (i - 1 != -1 || j - 1 != -1)
        {
            if (itable[i - 1][j - 1] == turn)
                count++;
            else
                break;
        }
        else
            break;
    }
    for (int i = row, j = col; i < 6, j < 7; i++, j++)
    {
        if (i + 1 != 6 || j + 1 != 7)
        {
            if (itable[i + 1][j + 1] == turn)
                count++;
            else
                break;
        }
        else
            break;
    }
    // return count >= 4;
    pthread_mutex_lock(&winner_mutex);
    if (*ptr_winner == 0)
    {
        *ptr_winner = count >= 4 ? turn : 0;
        if(*ptr_winner >= 4) printf("check_win_topLeft_bottomRight\n");
    }
    pthread_mutex_unlock(&winner_mutex);
    pthread_exit(NULL);
}

void check_win(int row, int col, int turn)
{
    int start, tids[NTHREADS_CHECK_WIN];
    pthread_t threads[NTHREADS_CHECK_WIN];
    pthread_attr_t attr;

    pthread_mutex_init(&winner_mutex, NULL);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < NTHREADS_CHECK_WIN; i++)
    {
        tids[i] = i;
        int thread_params[5];
        thread_params[0] = i;
        thread_params[1] = row;
        thread_params[2] = col;
        thread_params[3] = turn;
        if (i == 0)
        {
            pthread_create(&threads[i], &attr, check_win_bottom, (void *)&thread_params);
        }
        else if (i == 1)
        {
            pthread_create(&threads[i], &attr, check_win_left_right, (void *)&thread_params);
        }
        else if (i == 2)
        {
            pthread_create(&threads[i], &attr, check_win_topRight_bottomLeft, (void *)&thread_params);
        }
        else if (i == 3)
        {
            pthread_create(&threads[i], &attr, check_win_topLeft_bottomRight, (void *)&thread_params);
        }
    }

    for (int i = 0; i < NTHREADS_CHECK_WIN; i++)
    {
        pthread_join(threads[i], NULL);
    }

    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&winner_mutex);
}
