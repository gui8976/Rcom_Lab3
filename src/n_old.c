/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

typedef enum
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP_a,
} message_state;

message_state state = START;

void state_handler(unsigned char c)
{
    char buf[3];
    switch (state)
    {

    case START:
        if (c == 0x5C)
            state = FLAG_RCV;
        else
        {
            state = START;
            break;
        }
    case FLAG_RCV:
        if (c == 0x01 || c == 0x03)
        {
            state = A_RCV;
            buf[0] = c;
            break;
        }
        else if (c == 0x5C)
        {
            state = FLAG_RCV;
            break;
        }
        else
        {         
            state = START;
            break;
        }
    case A_RCV:
        if (c == 0x03)
        {
            buf[1] = c;
            state = C_RCV;
            break;
        }
        else if (c == 0x5C)
        {
            state = FLAG_RCV;
            break;
        }
        else
        {
            state = START;
            break;
        }

    case C_RCV:
        if (buf[0] ^ buf[1])
        {
            state = BCC_OK;
            break;
        }
        else if (c == 0x5C)
        {
            state = FLAG_RCV;
            break;
        }
        else
        {
            state = START;
            break;
        }

    case BCC_OK:
        if (c == 0x5C)
        {
            state = STOP_a;
            break;
        }
        else
        {
            state = START;
            break;
        }

    case STOP_a:
        break;
    }
}

int main(int argc, char **argv)
{
    int fd, c, res, i = 0;
    struct termios oldtio, newtio;
    char buff[255];

    if ((argc < 2) ||
        ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
         (strcmp("/dev/ttyS11", argv[1]) != 0)))
    {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */

    fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(argv[1]);
        exit(-1);
    }

    if (tcgetattr(fd, &oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 1;  /* blocking read until 5 chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");
    printf("New termios structure set\n");
    while (STOP == FALSE)
    {                           /* loop for input */
        res = read(fd, buff, 1); /* returns after 5 chars have been input */
        state_handler(buff[0]);
        if (state == STOP_a)
        {
            printf("Set state achieved!\n");
            char str[] = {0x5C, 0x03, 0x07, 0x06, 0x5C};
            str[3] = str[1]^str[2];
            write(fd, str, 255);
            break;
        }
    }
    // printf("writing Back: %s", str);
    sleep(1);
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    return 0;
}
