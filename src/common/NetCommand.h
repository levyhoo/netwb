#ifndef __NET_COMMAND__
#define __NET_COMMAND__


enum NetCommand
{
    NET_CMD_UNKOWN              = 0,
    NET_CMD_KEEPALIVE           = 1,
    NET_CMD_RPC                 = 2,    
    NET_CMD_KEEPALIVE_RESPONSE  = 3,
};

#endif
