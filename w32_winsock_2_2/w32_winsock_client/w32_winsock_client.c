//-----------------------------------------------------------------------------
//           Name: w32_winsock_client.c
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Simple TCP/UDP client using Winsock 2.2
//                 This client will attempt to connect with the server via 
//                 port 5001. If successful, it will then send a string to the 
//                 server and begin waiting for a response. Once the response
//                 returns, the client disconnects and shuts down.
//
//                 Before you try running this program, you'll need to launch
//                 the server executable located in the other workspace that
//                 comes with this sample project.
//-----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT  5001
#define DEFAULT_PROTOCOL SOCK_STREAM // TCP

int main() 
{
	WSADATA wsaData;
	SOCKET conn_socket;
	struct sockaddr_in server;
	struct hostent *hp;
	char buffer[128];
	char *server_name = "localhost";
	unsigned short port = DEFAULT_PORT;
	unsigned int addr = 0;
	int socket_type = DEFAULT_PROTOCOL;
	int retval = 0;
	int loopflag = 0;
	int loopcount = 0;
	int maxloop = -1;
	int i = 0;

	if( WSAStartup(0x202,&wsaData) == SOCKET_ERROR ) 
	{
		fprintf(stderr,"WSAStartup failed with error %d\n",WSAGetLastError());
		WSACleanup();
		return -1;
	}
	
	// Attempt to detect if we should call gethostbyname() or
	// gethostbyaddr()

	if( isalpha(server_name[0]) ) 
	{   
		// server address is a name
		hp = gethostbyname(server_name);
	}
	else 
	{ 
		// Convert nnn.nnn address to a usable one
		addr = inet_addr(server_name);
		hp = gethostbyaddr((char *)&addr,4,AF_INET);
	}

	if( hp == NULL ) 
	{
		fprintf(stderr,"Client: Cannot resolve address [%s]: Error %d\n", 
			server_name,WSAGetLastError());
		WSACleanup();
		exit(1);
	}

	// Copy the resolved information into the sockaddr_in structure
	memset(&server,0,sizeof(server));
	memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = htons(port);

	conn_socket = socket(AF_INET,socket_type,0); /* Open a socket */

	if( conn_socket < 0 ) 
	{
		fprintf(stderr,"Client: Error Opening socket: Error %d\n",
			WSAGetLastError());
		WSACleanup();
		return -1;
	}

	// Notice that nothing in this code is specific to whether we 
	// are using UDP or TCP.
	// We achieve this by using a simple trick.
	//    When connect() is called on a datagram socket, it does not 
	//    actually establish the connection as a stream (TCP) socket
	//    would. Instead, TCP/IP establishes the remote half of the
	//    ( LocalIPAddress, LocalPort, RemoteIP, RemotePort) mapping.
	//    This enables us to use send() and recv() on datagram sockets,
	//    instead of recvfrom() and sendto()

	printf("Client connecting to: %s\n",hp->h_name);

	if(connect(conn_socket,(struct sockaddr*)&server,sizeof(server)) == SOCKET_ERROR) 
	{
		fprintf(stderr,"connect() failed: %d\n",WSAGetLastError());
		fprintf(stderr,"Server not found...\n\n");
		WSACleanup();
		return -1;
	}

	// cook up a string to send
	loopcount = 0;

	while(1) 
	{
		wsprintf( buffer,"This is a small test message [number %d]",loopcount++ );

		retval = send(conn_socket,buffer,sizeof(buffer),0);

		if( retval == SOCKET_ERROR ) 
		{
			fprintf(stderr,"send() failed: error %d\n",WSAGetLastError());
			WSACleanup();
			return -1;
		}

		printf("Sent Data [%s]\n",buffer);
		retval = recv(conn_socket,buffer,sizeof (buffer),0 );

		if( retval == SOCKET_ERROR ) 
		{
			fprintf(stderr,"recv() failed: error %d\n",WSAGetLastError());
			closesocket(conn_socket);
			WSACleanup();
			return -1;
		}

		// We are not likely to see this with UDP, since there is no
		// 'connection' established. 
		if( retval == 0 ) 
		{
			printf("Server closed connection\n");
			closesocket(conn_socket);
			WSACleanup();
			return -1;
		}

		printf("Received %d bytes, data [%s] from server\n",retval,buffer);

		if( !loopflag )
		{
			printf("Terminating connection\n");
			break;
		}
		else 
		{
			if( (loopcount >= maxloop) && (maxloop > 0) )
				break;
		}
	}

	closesocket(conn_socket);
	WSACleanup();

	return 0;
}
