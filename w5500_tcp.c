
#include "socket.h"
#include "w5500.h"

#define DATA_BUF_SIZE			2048

int32_t tcp_client(uint8_t sn, uint8_t* destip, uint16_t destport)
{
    int32_t ret; // return value for SOCK_ERRORs
    switch(getSn_SR(sn))
    {
        case SOCK_ESTABLISHED :
            if(getSn_IR(sn) & Sn_IR_CON)	// Socket n interrupt register mask; TCP CON interrupt = connection with peer is successful
            {
                setSn_IR(sn, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
            }
            return SOCK_ESTABLISHED;
        break;
        case SOCK_CLOSE_WAIT :
            if((ret=disconnect(sn)) != SOCK_OK) return ret;
            break;
        case SOCK_INIT :
            if( (ret = connect(sn, destip, destport)) != SOCK_OK) return ret;	//	Try to TCP connect to the TCP server (destination)
        break;

        case SOCK_CLOSED:
            close(sn);
            if((ret=socket(sn, Sn_MR_TCP, destport, 0x00)) != sn) return ret; // TCP socket open with 'any_port' port number
            break;
        default:
            return 0;
            break;
    }
    return 0;
}

// int32_t tcp_server(uint8_t sn, uint8_t* buf, uint16_t port)
int32_t tcp_server(uint8_t sn, uint16_t port)
{
   int32_t ret;
   uint16_t size = 0, sentsize=0;
#ifdef _LOOPBACK_DEBUG_
   uint8_t destip[4];
   uint16_t destport;
#endif

   switch(getSn_SR(sn))
   {
      case SOCK_ESTABLISHED :
         if(getSn_IR(sn) & Sn_IR_CON)
         {
            if(getSn_IR(sn) & Sn_IR_CON) {
                setSn_IR(sn, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
            }

#ifdef _LOOPBACK_DEBUG_
        	 getSn_DIPR(sn, destip);
        	 destport = getSn_DPORT(sn);

        	 printf("%d:Connected - %d.%d.%d.%d : %d\r\n",sn, destip[0], destip[1], destip[2], destip[3], destport);

            setSn_IR(sn,Sn_IR_CON);
         }
         if((size = getSn_RX_RSR(sn)) > 0)
         {
            if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
            ret = recv(sn, buf, size);
            if(ret <= 0) return ret;
            sentsize = 0;
            while(size != sentsize)
            {
               ret = send(sn,buf+sentsize,size-sentsize);
               if(ret < 0)
               {
                  close(sn);
                  return ret;
               }
               sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
#endif
         }
         return SOCK_ESTABLISHED;
         break;
      case SOCK_CLOSE_WAIT :
#ifdef _LOOPBACK_DEBUG_
         //printf("%d:CloseWait\r\n",sn);
#endif
         if((ret=disconnect(sn)) != SOCK_OK) return ret;
#ifdef _LOOPBACK_DEBUG_
         printf("%d:Socket closed\r\n",sn);
#endif
         break;
      case SOCK_INIT :
#ifdef _LOOPBACK_DEBUG_
    	 printf("%d:Listen, TCP server loopback, port [%d]\r\n",sn, port);
#endif
         if( (ret = listen(sn)) != SOCK_OK) return ret;
         break;
      case SOCK_CLOSED:
#ifdef _LOOPBACK_DEBUG_
         //printf("%d:TCP server loopback start\r\n",sn);
#endif
         if((ret=socket(sn, Sn_MR_TCP, port, 0x00)) != sn)
         //if((ret=socket(sn, Sn_MR_TCP, port, Sn_MR_ND)) != sn)
            return ret;
#ifdef _LOOPBACK_DEBUG_
         //printf("%d:Socket opened\r\n",sn);
#endif
         break;
      default:
         break;
   }
   return 1;
}

int32_t rcvfromServer(uint8_t sn, uint8_t* buf)
{
	int32_t s_stat,ret;
    uint16_t size = 0;
    s_stat = getSn_SR(sn);
    if(s_stat == SOCK_ESTABLISHED) {
        if(getSn_IR(sn) & Sn_IR_CON) {
            setSn_IR(sn, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
        }
        if((size = getSn_RX_RSR(sn)) > 0) // Sn_RX_RSR: Socket n Received Size Register, Receiving data length
        {
            if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE; // DATA_BUF_SIZE means user defined buffer size (array)
            ret = recv(sn, buf, size); // Data Receive process (H/W Rx socket buffer -> User's buffer)
            return ret; // If the received data length <= 0, receive failed and process end
        }
    }
    return 0;
}

int32_t sendtoServer(uint8_t sn, uint8_t* buf, uint16_t len)
{
	int32_t s_stat,ret;
    s_stat = getSn_SR(sn);
    if(s_stat == SOCK_ESTABLISHED) {
        if(getSn_IR(sn) & Sn_IR_CON) {
            setSn_IR(sn, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
        }
        ret = send(sn, (uint8_t *)buf, len);
        if(ret < 0){
            close(sn);
            return ret;
        }
	}
	return 0;
}
