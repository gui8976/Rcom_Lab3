#include "linklayer.h"


// LREAD E LCLOSE ESTÃO BEM (necessitam de uma olhadela) 
//O LWRITE ESTA BEM FALTA IMPLEMETAR AS MESSAGENS RR 
// O LREAD FALTA CALCULAR O BCC 2 E IMPLEMENTAR AS MESSAGENS RR

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define F 0x5C
#define A1 0x01
#define A2 0x03
#define C_SET 0x03
#define C_UA 0x07
#define C_RR 0X1
#define C_RJ 0X101

/*!!!!!!!!!!!!!!!!!!!!!!!! FALTA IMPLEMETAR !!!!!!!!!!!!!!!!!!*/
//#define C_NS_0 0x00
//#define C_NS_1 0x02
//#define C_RR_0 0x01
//#define C_RR_1 0x21
/*!!!!!!!!!!!!!!!!!!!!!!!! FALTA IMPLEMETAR !!!!!!!!!!!!!!!!!!*/

#define ESC 0X5D
#define DSC 0x0B
#define SET_1 \
    { F, A1, C_SET, A1 ^ C_SET, F }

#define UA_1 \
    { F, A1, C_UA, A1 ^ C_UA, F }

#define RR \
    { F, A1, C_RR, A1 ^ C_RR, F }

#define RJ \
    { F, A1, C_RJ, A1 ^ C_RJ, F }
#define DC \
    { F, A1, DSC, A1 ^ DSC, F }

int alarm_count = 0;
int fd;
volatile int flag_lwrite =0;
volatile int flag_read =0;

bool alarm_flag = false;
typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP_a,
} message_state;

message_state state = START;

void state_handler_SET(unsigned char c) {
    char buf[3];
    switch (state) {
        case START:
            if (c == F)
                state = FLAG_RCV;
            else {
                state = START;
                break;
            }
        case FLAG_RCV:
            if (c == A1 || c == A2) {
                state = A_RCV;
                buf[0] = c;
                break;
            } else if (c == F) {
                state = FLAG_RCV;
                break;
            } else {
                state = START;
                break;
            }

        case A_RCV:
            if (c == C_SET) {
                buf[1] = c;
                state = C_RCV;
                break;
            } else if (c == F) {
                state = FLAG_RCV;
                break;
            } else {
                state = START;
                break;
            }

        case C_RCV:
            if (A1 ^ C_SET == buf[0] ^ buf[1]) {
                state = BCC_OK;
                break;
            } else if (c == F) {
                state = FLAG_RCV;
                break;
            } else {
                state = START;
                break;
            }

        case BCC_OK:
            if (c == F) {
                state = STOP_a;
                break;
            } else {
                state = START;
                break;
            }

        case STOP_a:
            break;
    }
}

void state_handler_UA(unsigned char c) {
    char buf[3];
    switch (state) {
        case START:
            if (c == F)
                state = FLAG_RCV;
            else {
                state = START;
                break;
            }
        case FLAG_RCV:
            if (c == A1 || c == A2) {
                state = A_RCV;
                buf[0] = c;
                break;
            } else if (c == F) {
                state = FLAG_RCV;
                break;
            } else {
                state = START;
                break;
            }
        case A_RCV:
            if (c == C_UA) {
                buf[1] = c;
                state = C_RCV;
                break;
            } else if (c == F) {
                state = FLAG_RCV;
                break;
            } else {
                state = START;
                break;
            }

        case C_RCV:
            if (A1 ^ C_UA == buf[0] ^ buf[1]) {
                state = BCC_OK;
                break;
            } else if (c == F) {
                state = FLAG_RCV;
                break;
            } else {
                state = START;
                break;
            }

        case BCC_OK:
            if (c == F) {
                state = STOP_a;
                break;
            } else {
                state = START;
                break;
            }

        case STOP_a:
            break;
    }
}

void state_handler_DC(unsigned char c) {
    char buf[3];
    switch (state) {
        case START:
            if (c == F)
                state = FLAG_RCV;
            else {
                state = START;
            }
            break;
        case FLAG_RCV:
            if (c == A1 || c == A2) {
                state = A_RCV;
                buf[0] = c;

            } else if (c == F) {
                state = FLAG_RCV;

            } else {
                state = START;
            }
            break;
        case A_RCV:
            if (c == DSC) {
                buf[1] = c;
                state = C_RCV;

            } else if (c == F) {
                state = FLAG_RCV;

            } else {
                state = START;
            }
            break;
        case C_RCV:
            if (A1 ^ DSC == buf[0] ^ buf[1]) {
                state = BCC_OK;

            } else if (c == F) {
                state = FLAG_RCV;

            } else {
                state = START;
            }
            break;
        case BCC_OK:
            if (c == F) {
                state = STOP_a;

            } else {
                state = START;
            }
            break;
        case STOP_a:
            break;
    }
}

void state_handler_lread(unsigned char c) {
    char buf[3];
    switch (state) {
        case START:
            if (c == F)
                state = FLAG_RCV;
            else {
                state = START;
            }
            break;
        case FLAG_RCV:
            if (c == A1 || c == A2) {
                state = A_RCV;
                buf[0] = c;

            } else if (c == F) {
                state = FLAG_RCV;

            } else {
                state = START;
            }
            break;
        case A_RCV:
            if (c == C_SET) {
                buf[1] = c;
                state = C_RCV;
            } else if (c == F) {
                state = FLAG_RCV;

            } else {
                state = START;
            }
            break;
        case C_RCV:
            if (A1 ^flag_read == buf[0] ^ buf[1]) {
                state = BCC_OK;
            } else if (c == F) {
                state = FLAG_RCV;
            } else {
                state = START;
            }
            break;
        case BCC_OK:
            if (c = F) {
                state = STOP_a;
            }
            break;
    }
}

/*void state_handler_RR(unsigned char c)
{

} */


void atende() {
    alarm_count++;
    alarm_flag = true;
    printf("alarme\n");
}

int llopen(linkLayer connectionParameters) {
    int fd;
    struct termios oldtio, newtio;

    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);

    if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE_DEFAULT | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    if (connectionParameters.role != 0 && connectionParameters.role != 1)
        return -1;

    if (connectionParameters.role)  // Receiver
    {
        int bytes_read = 0;
        char buff;

        while (true) {
            bytes_read = read(fd, &buff, sizeof(buff));
            if (!bytes_read) break;
            state_handler_SET(buff);
            if (state == STOP_a) {
                printf("Set state achieved!\n");
                char str[] = UA_1;
                write(fd, str, sizeof str);
                return 1;
            }
        }
    }
    if (!connectionParameters.role)  // Transmitter
    {
        char str;

        char buf[] = SET_1;

        alarm_count = 0;
        alarm_flag = false;

        (void)signal(SIGALRM, atende);

        while (alarm_count < connectionParameters.numTries) {
            alarm_flag = false;
            alarm(connectionParameters.timeOut);
            write(fd, buf, strlen(buf) + 1);

            while (true) {
                read(fd, &str, sizeof(str));
                state_handler_UA(str);
                if (alarm_flag)
                    return -1;
                if (state == STOP_a) {
                    printf("UA state achieved!\n");
                    alarm(0);
                    return 1;
                }
            }
            if (state == STOP_a)
                break;
            printf("timeout in %d\n", alarm_count);
        }

    }
}

int llwrite(char* buf, int bufSize) {  // stuffing
    int res;
    char array[2000] = {F, A1, C_SET};

    char BCC2 = buf[0];

    char aux[2000];

    char buf;

    int array_pos = 0;


    if (bufSize > 1) {
        for (int i = 1; i < bufSize; i++) {
            BCC2 = buf[i] ^ BCC2;
        }
    }
    char flag[3] = {BCC2, F};

    for (int i = 0; i < bufSize; i++) {
        if (buf[i] == 0x5C) {
            aux[array_pos] = ESC;
            aux[array_pos + 1] = 0x07C;
            array_pos += 2;
        } else if (buf[i] == 0x5D) {
            aux[array_pos] = ESC;
            aux[array_pos + 1] = 0x07D;
            array_pos += 2;
        } else {
            aux[array_pos] = buf[i];
            array_pos++;
        }
    }
    strcat(array, aux);
    strcat(array, flag);

    write(fd, array, sizeof array);
 

    alarm(3);
/*  
    while(true){
        res = read(fd,&buf , sizeof buf);
        // maquina de estados do RR 
    }
    alarm(0);

    alarm_count=0;


    flag_lwrite = 1;
 */
    return 1;
}

int llread(char* packet) {  // destuffing
    int i = 0,bytes_read =0;
    int bytes_read = 0;

    char buf, aux[2000];

    while (true) {
        bytes_read = read(fd, &buf, sizeof buf);
        state_handler_lread(buf);
        if (state == BCC_OK) {
            if (i == 0) {
                aux[i] = buf;
                i++;
            }
            if ((aux[i] == 0x7D && aux[i - 1] == ESC))
                continue;
            else if ((aux[i] == 0x7C && aux[i - 1] == ESC)) {
                aux[i - 1] = 0x5C;
                continue;
            } else {
                aux[i] = buf;
                i++;
            };
        }
        if (state == STOP_a)
            break;
    }
    for ( int j = 4; j< i-2; j++)
        packet[j-4] = aux[j];

// calcular o Bcc 2 e dar check para ver se é verdade 
    if( aux[i-1]== aux[i-2]^aux[i-3])
        printf("Frame read OK\n");
    else printf("ERROR reading frame\n");

 printf("\n\n --- DESTUFFED DATA ---\n\n");
// envia o RR

    bytes read = i-6;
    return bytes_read;
}


int llclose(linkLayer connectionParameters, int showStatistics) {
    struct termios oldtio;

    if (!connectionParameters.role) {
        sleep(1);
        char disc[] = DC;
        char UA[] = UA_1;
        char str;

        int bytes_read = 0;
        write(fd, disc, sizeof(disc));
        alarm(connectionParameters.timeOut);

        bytes_read = read(fd, &str, sizeof str);

        while (bytes_read)
            state_handler_DC(str);

        if (state == STOP_a)
            alarm(0);

        write(fd, UA, sizeof(UA));

        printf("\n\n ------ TERMINATING------\n\n");

        if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
            perror("tcsetattr");
            exit(-1);
        }
        close(fd);
        return 1;
    }

    if (connectionParameters.role) {  // desconectar o receiver
        sleep(1);
        char disc[] = DC;
        char UA[] = UA_1;
        char str;

        int bytes_read = 0;

        bytes_read = read(fd, &str, sizeof str);

        while (bytes_read)
            state_handler_DC(str);

        write(fd, disc, sizeof(disc));
        alarm(connectionParameters.timeOut);

        bytes_read = read(fd, &str, sizeof str);

        while (bytes_read)
            state_handler_UA(str);

        if (state == STOP_a)
            alarm(0);

        printf("\n\n ------ TERMINATING------\n\n");

        if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
            perror("tcsetattr");
            exit(-1);
        }
        close(fd);
        return 1;
    }
    return 0;
}