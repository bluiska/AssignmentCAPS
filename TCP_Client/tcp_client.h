#ifndef __TCPCLIENT_H
#define __TCPCLIENT_H

#include <winsock2.h>
#include <string>

class TCPClient
{
public:
	TCPClient(std::string server, unsigned short int port);
	~TCPClient();

	std::string send(std::string request) const;
	void OpenConnection();
	void CloseConnection();

private:
	std::string server;
	unsigned short int port;
	std::string portString;

	SOCKET ConnectSocket;
};

#endif // __TCPCLIENT_H
