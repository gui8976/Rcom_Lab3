/*Non-Canonical Input Processing*/

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

#include "variaveis.h"

int alarm_count = 0;
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

void state_handler(unsigned char c) {
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
            if (buf[0] ^ buf[1]) {
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

void atende() {
    alarm_count++;
    alarm_flag = true;
    printf("alarme\n");
}
int main(int argc, char** argv) {
    int c, fd;
    char str;
    struct termios oldtio, newtio;

    (void)signal(SIGALRM, atende);

    if (argc < 2) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */

    fd = open(argv[1], O_RDWR | O_NOCTTY);

    if (fd < 0) {
        perror(argv[1]);
        exit(-1);
    }

    if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    char buf[] = SET_1;
    alarm_count = 0;
    alarm_flag = false;
    while (alarm_count < 3) {
        alarm_flag = false;
        alarm(3);
        write(fd, buf, strlen(buf) + 1);      
        while (true) {
            read(fd, &str, sizeof(str));
            state_handler(str);
            if (alarm_flag)
                break;
            if (state == STOP_a) {
                printf("UA state achieved!\n");
                alarm(0);
                break;
            }
        }
        if (state == STOP_a)
            break;
        printf("timeout in %d\n", alarm_count);
    }


    sleep(1);

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }
    close(fd);
    return 0;
}
