#ifndef inc_COM_CONST
#define inc_COM_CONST

#define MAX_MSG_SIZE 512
#define MAX_POSN 15
#define MSG_TYPE_SIZE 2
#define SRC_PORT_SIZE 2
#define NUMBER_PIGS_SIZE 2
#define NUMBER_WALLS_SIZE 2
#define OTHER_PORT_SIZE 2
#define PORT_LOC_SIZE 2
#define WALL_LOC_SIZE 2
#define HOP_LIMIT_SIZE 2
#define COM_IP_ADDR "127.0.0.2"
#define COORD_LISTEN_PORT 24544
#define MAX_PORT 65535

enum MessageTypes
{
  INIT_POSN_MSG=1
};

#endif
