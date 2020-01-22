#ifndef __TCPSERVER_H
#define __TCPSERVER_H

#include "tcp_received_socket_data.h"

class TCPServer
{
public:
	TCPServer(unsigned short int port);
	~TCPServer();
	ReceivedSocketData accept();
	void receiveData(ReceivedSocketData& ret, bool blocking);
	int sendReply(ReceivedSocketData reply);

	void OpenListenSocket();
	void CloseListenSocket();
	int closeClientSocket(ReceivedSocketData &reply);

private:
	SOCKET ListenSocket;
	unsigned short int port;
	std::string portString;
};

#endif __TCPSERVER_H
