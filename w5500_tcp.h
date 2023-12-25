#ifndef W5500_TCP_H
#define W5500_TCP_H

// int32_t tcp_server(uint8_t sn, uint8_t* buf, uint16_t port);
int32_t tcp_server(uint8_t sn, uint16_t port);
int32_t tcp_client(uint8_t sn, uint8_t* destip, uint16_t destport);
int32_t rcvfromServer(uint8_t sn, uint8_t* buf);
int32_t sendtoServer(uint8_t sn, uint8_t* buf, uint16_t len);

#define rcvfrom rcvfromServer
#define sendTo  sendtoServer
#endif
