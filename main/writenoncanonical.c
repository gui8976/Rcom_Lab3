/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

#define F 0x5C
#define A1 0x01  
#define A2 0x03  
#define C_SET 0x03
#define C_UA 0x07

volatile int STOP = FALSE;
#define SET_1 \
    { F, A1, C_SET, A1 ^ C_SET, F }

#define SET_2 \
    { F, A2, C_SET, A1 ^ C_SET, F }

#define UA_1 \
    { F, A1, C_UA, A1 ^ C_UA, F }

#define UA_2 \
    { F, A2, C_UA, A2 ^ C_UA, F }

int alarm_count = 0;
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
        if (c == F)
            state = FLAG_RCV;
        else
        {
            state = START;
            break;
        }
    case FLAG_RCV:
        if (c == A1 || c == A2)
        {
            state = A_RCV;
            buf[0] = c;
            break;
        }
        else if (c == F)
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
        if (c == C_UA)
        {
            buf[1] = c;
            state = C_RCV;
            break;
        }
        else if (c == F)
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
        else if (c == F)
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
        if (c == F)
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


void atende()
{
    //char buf[]= {0x5C, 0x01, 0x03, 1, 0x5C };
    //buf[3]=buf[1]^buf[2];
    //write(fd,buf,strlen(buf)+1);
    alarm_count++;
    if(alarm > 3){ }
        exit(1);
    alarm(3);
}
int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255], str[255];
    (void) signal(SIGALRM, atende);

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS10", argv[1])!=0) &&
          (strcmp("/dev/ttyS11", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }


    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd < 0) { perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */


    tcflush(fd, TCIOFLUSH);
    

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    //char buf[]= {0x5C, 0x01, 0x03, 1, 0x5C };
    //buf[3]=buf[1]^buf[2];

    //write(fd,buf,strlen(buf)+1);
    alarm(3);

    while (STOP==FALSE) {    
             
        read(fd, buf, 1); 
        state_handler(buf[0]);
        if (state == STOP_a)
        {
            printf("UA state achieved!\n");
            alarm(0);
            break;
        }
    }
    alarm(0);
    
    sleep(1);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }
    close(fd);
    return 0;
}
