#ifndef __VARIAVEIS_H__
#define __VARIAVEIS_H__

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define F 0x5C
#define A1 0x01
#define A2 0x03
#define C_SET 0x03
#define C_UA 0x07

#define SET_1 \
    { F, A1, C_SET, A1 ^ C_SET, F }

#define SET_2 \
    { F, A2, C_SET, A2 ^ C_SET, F }

#define UA_1 \
    { F, A1, C_UA, A1 ^ C_UA, F }

#define UA_2 \
    { F, A2, C_UA, A2 ^ C_UA, F }
#endif
