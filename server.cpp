#include <iostream>
#include <string>
using namespace std;

#include <time.h>
#include <Winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

/* Function to prepare to receive connections for a service. */
static SOCKET passiveTCP(string service, int qlen);

int main(int argc, char* argv[])
{
	// Make sure that we can connect:
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {


		cerr << "Could not find useable DLL" << endl;
		return 1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {

		// Tell the user that we couldn't find a useable winsock DLL. 
		cerr << "Could not find useable DLL" << endl;
		WSACleanup();  // Terminates the use of the library. 
		return 1;
	}
	string service = "7007"; // The default port number for the service to be provided.

	/* Fill in the remote host name and service name from the run-time
	* parameters. */
	switch (argc) {

		/* If there are no run-time parameters the default values will be
		* used. */
	case 1:
		break;

		/* Record the host name. */
	case 2:
		service = argv[1];
		break;

		/* If there are more than 1 run-time parameters, the user is in
		* error. */
	default:
		fprintf(stderr, "Usage: server [service]\n");
		WSACleanup();
		exit(1);
	}
	/* Connect to the server process. */
	SOCKET ls = passiveTCP(service, 5);

	fd_set cliSockets;
	fd_set copy;
	string user;

	const int CLIENTS = 100;

	int total;
	int new_socket;
	int client_socket[CLIENTS] = { 0 };
	int max_clients = 100;
	int messageSize = 0;
	char buffer[1025];
	char username[100];
	char password[100];
	char status[50] = "ok\n";
	int sd;
	int cd;
	FD_ZERO(&cliSockets);
	
	
	

	/* Wait for connections from clients. */
	while (true) {

		FD_ZERO(&cliSockets);
		FD_SET(ls, &cliSockets);
		for (int i = 0; i < max_clients; i++) 
		{
			if (client_socket[i] != 0) 
			{
				FD_SET(client_socket[i], &cliSockets);
			}
		}

		
		
		
		

		struct sockaddr_in fsin; // address of client
		int alen = sizeof(sockaddr_in); // length of client address


		if ((total = select(0, &cliSockets, NULL, NULL, NULL)) == SOCKET_ERROR) {
			cout << "select() returned with error %d\n" << WSAGetLastError() << endl;
		}
		else if (total == 0) {
			cout << "nobody here.";
		}
		else {
			cout << "select() returned without an error %d\n" << endl;
		}

		

		if (FD_ISSET(ls, &cliSockets)) 
		{
			if ((new_socket = accept(ls, (struct sockaddr*) & fsin, &alen)) < 0) {
				cout << "Socket not Accepted?" << endl;
				exit(EXIT_FAILURE);

			}
			else
			{
				FD_SET(new_socket, &cliSockets);
				
				// add the new client socket to the array of sockets
				for (int i = 0; i < max_clients; i++) {
					if (new_socket == NULL || client_socket[i] == new_socket) {
						break;
					}
					if (client_socket[i] == 0) {
						recv(new_socket, username, 100, 0);
						recv(new_socket, password, 100, 0);

						//authenticate(username, password);

						if (send(new_socket, status, 2, 0) >= 0) {
							client_socket[i] = new_socket;
							cout << new_socket << " value.";
							break;
						}
					}
				}
			}
		}
		

		

		

		for (int i = 0; i < max_clients; i++) {
			sd = client_socket[i];

			if (FD_ISSET(sd, &cliSockets)) {

				int bytesRead = 0;
				messageSize = 0;

				messageSize = recv(sd, buffer, 1024, 0);
				
				
				if (messageSize < 0) 
				{
					printf("recv failed: %d\n", WSAGetLastError());
					closesocket(sd);
					client_socket[i] = 0;
					break;
				}
				if (messageSize == 0)
				{
					printf("Disconnect on recv");
					closesocket(sd);
					client_socket[i] = 0;
					continue;
				}

				 


				cout << "received.";
				cout << "sending";
				user = "User " + to_string(client_socket[i]) + ": ";
				
				buffer[messageSize] = '\0';
				for (int k = 0; k < max_clients; k++) 
				{
					cd = client_socket[k];

					if (cd != 0) 
					{
						send(cd, user.c_str(), 9, 0);
						int srvh = send(cd, buffer, messageSize, 0);

						if (srvh <= 0)
						{
							cerr << "Send Failed" << endl;
							closesocket(sd);
							client_socket[i] = 0;
						}
					}
				}
				
				/*int srvh = send(sd, buffer, messageSize, 0);

				if (srvh <= 0)
				{
					cerr << "Send Failed" << endl;
					closesocket(sd);
					client_socket[i] = 0;
				}
				*/

				

				//closesocket(sd);
				//client_socket[i] = 0;



			}
			else {

			}
			
		}

		

	}
}

static SOCKET passiveTCP(string service, int qlen)

{
	struct servent* pse; /* Points to service information. */
	struct sockaddr_in sin; /* Internet endpoint address. */

	memset((char*)&sin, 0, sizeof(sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	/* Get the port number for the service. */
	if ((pse = getservbyname(service.c_str(), "tcp")) != NULL)
	{
		sin.sin_port = (u_short)pse->s_port;
	}
	else if ((sin.sin_port = htons((u_short)stoi(service))) == 0)
	{
		cerr << "Bad Port number/service specified: " << service << endl;;
		exit(1);
	}
	/* Allocate a socket. */
	SOCKET ls;
	if ((ls = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		int errorcode = WSAGetLastError();
		cerr << "socket call failed: " << errorcode << endl;
		exit(1);
	}
	/* Bind the address to the socket. */
	if (bind(ls, (struct sockaddr*) & sin, sizeof(sin)) == INVALID_SOCKET)
	{
		int errorcode = WSAGetLastError();
		cerr << "bind call failed: " << errorcode << endl;
		exit(1);
	}
	// Indicate that we are ready to wait for connects.
	if (listen(ls, qlen) == INVALID_SOCKET)
	{
		int errorcode = WSAGetLastError();
		cerr << "listen call failed: " << errorcode << endl;
		exit(1);
	}
	// Return the listening socket. 
	return ls;
}