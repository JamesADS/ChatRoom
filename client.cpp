#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")  // Weird what happends when I forgot the closing paren.

#include <process.h>
#include <iostream>
#include <time.h>
#include <string>
using namespace std;


#include <Winsock2.h>

void displayProcess();

string msgtyped;
char mtyped[1022];
SOCKET chatty;

const int LINESZ = 2000;

static SOCKET connectTCP(const char* host, const char* service);
unsigned _stdcall passMessages(void* alsoDuck);
unsigned _stdcall typeMessages(void* duck);

int main(int argc, char* argv[])
{

	// Make sure that we can connect:
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	// A macro to create a word with low order and high order bytes as specified.
	wVersionRequested = MAKEWORD(1, 1);

	// Test that version 1.1 is available.
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {

		// Tell the user that we couldn't find a useable winsock.dll. */
		cerr << "Could not find useable DLL" << endl;
		return 1;
	}

	// Confirm that the Windows Sockets DLL supports version 1.1.
	// Note that if the DLL supports versions greater than 1.1 in addition 
	//  to 1.1, it will still return1.1 in wVersion since that is the version we
	// requested.  Note the use of macros.  Furthermore for version 1.1
	// this check is not really needed.

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		// Tell the user that we couldn't find a useable winsock DLL. 
		WSACleanup();  // Terminates the use of the 
		cerr << "Could not find useable DLL" << endl;
		return 1;
	}


	string host = "localhost";
	string service = "7007";
	string dc = "7009";

	SOCKET s;       /* The socket for the connection to the remote host. */
	int nb;         /* The number of bytes read from a single read. */
	int tnb;        /* The total number of bytes read. */
	int sb;  /*Total number of sent bytes.*/
	char buff[LINESZ + 1];  /* Response data from the server. */

	/* Fill in the remote host name and service name from the run-time
	 * parameters. */
	switch (argc) {

		/* If there are no run-time parameters the default values will be
		 * used. */
	case 1:
		break;

		/* Record the host name. */
	case 2:
		host = argv[1];
		break;

		/* If there are two run-time parameters, record the service name
		 * and the host name. */
	case 3:
		host = argv[1];
		service = argv[2];
		break;

		/* If there are more than 2 run-time parameters, the user is in
		 * error. */
	default:
		fprintf(stderr, "Usage: client [host [service]]\n");
		return 1;
	}

	/* Connect to the server process. */
	s = connectTCP(host.c_str(), service.c_str());
	string username;
	string password;
	char buffer[1024];
	int size = 0;
	string status;


	HANDLE hthread[2];

	cout << "Enter username: ";
	cin >> username;
	system("cls");
	cout << "Enter password: ";
	cin >> password;

	send(s, username.c_str(), username.length(), 0);
	send(s, password.c_str(), password.length(), 0);

	while (size == 0) 
	{
		size = recv(s, buffer, 1023, 0);
	}
	//buffer = [ok] on success
	//buffer = [no] on failure

	for (int i = 0; i < size; i++)
	{
		status += buffer[i];
	}

	if (status == "ok")
	{
		cout << "received ok status.";
		displayProcess();
	}
	else
	{
		cerr << "failure to connect to server.";
		exit(1);
	}

	chatty = connectTCP(host.c_str(), dc.c_str());

	//send messages to server, receive messages from server and pass to DC
	hthread[0] = (HANDLE)_beginthreadex(NULL, 0, typeMessages, (void*)s, 0, NULL);
	hthread[1] = (HANDLE)_beginthreadex(NULL, 0, passMessages, (void*)s, 0, NULL);

	if (WaitForMultipleObjects(2, hthread, TRUE, INFINITE) == WAIT_FAILED) {

		cerr << "WaitForMultipleObjects failed.  err = " << GetLastError() << endl;
		return 1;
	}

	return 0;
}

unsigned _stdcall typeMessages(void* duck)
{
	SOCKET s = (SOCKET)duck;
	while (true)
	{
		getline(cin, msgtyped);

		if (msgtyped.length() > 256) {
			cout << "Message too long. Please limit to 256 characters." << endl;
			continue;
		}
		
		for (int i = 0; i < msgtyped.size(); i++) 
		{
			mtyped[i] = msgtyped[i];
		}

		send(s, mtyped, msgtyped.size(), 0);


	}

	return 0;


}

unsigned _stdcall passMessages(void* alsoDuck)
{
	char buff[1025];
	SOCKET s = (SOCKET)alsoDuck;
	while (true)
	{
		int messageSize = recv(s, buff, 1022, 0);

		
		buff[messageSize] = '\0';
		send(chatty, buff, messageSize, 0);

	}
}


void displayProcess()
{
	STARTUPINFOA si;

	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));

	si.cb = sizeof(si);

	ZeroMemory(&pi, sizeof(pi));

	// Not that we are creating a new console window.  

	if (!CreateProcess("C:\\Users\\JP\\source\\repos\\client2\\Debug\\DisplayChat.exe"

		, NULL, NULL, NULL, false, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))

	{

		cerr << "CreateProcessA - failed" << endl;

		return;

	}
	return;
}

/**/
/*
NAME

	connectTCP - to make a connection to a TCP/IP server.

SYNOPSIS
*/
static SOCKET connectTCP(const char* host, const char* service)
/*
DESCRIPTION

	The "connectTCP" function will connect this client to a TCP/IP server.
	"host" is the name or IP address of the system on which the server
	resides.  "service" is the name or port number of the service to be
	accessed.

RETURNS

	This function returns the file descriptor of the connected socket
	if it is successful and does not return if it fails.  The error
	is displayed in the latter case.
*/
/**/
{
	struct hostent* he; /* Information about the remote host. */
	struct servent* se; /* Information about the requested service. */
	struct sockaddr_in sin; /* Internet end point address.  How do I find the format of this struct? */
	SOCKET s;              /* File descriptor for the connected socket. */


	/* Create an endpoint address for the server.   Structure for address
	 * found in: netinet/in.h */
	memset((char*)&sin, 0, sizeof(sin)); /* sets the address to zero. */
	sin.sin_family = AF_INET;  /* Record the fact that we are using the
								* internet family of addresses. */

								/* Get the portnumber.  Note: services found in /etc/services. */
	if ((se = getservbyname(service, "tcp")) != NULL)
	{
		sin.sin_port = se->s_port;
		printf("port No. %ld\n", (long int)ntohs(se->s_port));
	}
	else if ((sin.sin_port = htons((short int)atoi(service)))
		== 0)
	{
		printf("Can't get the port number of the service.\n");
		exit(1);
	}

	/* Get the address of the remote host.  The host's name is looked up
	 * on a name server. */
	if ((he = gethostbyname(host)) != NULL)
	{
		memcpy((char*)&sin.sin_addr, he->h_addr, he->h_length);
	}
	else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
	{
		printf("Invalid host address\n");
		exit(1);
	}
	// Display the IP address in hexadecimal.  Let's convert it to dotted 
	// notation in class.
	printf("IP address = 0x%x\n", sin.sin_addr.s_addr);

	/* Allocate a socket from which the connection to the server will be
	 * be made.  Note: by setting the third argument to 0, means that we
	 * are using the default protocol (since there is usually one choice,
	 * 0 seems like a good choice for the argument. */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not allocate socket: %d\n", WSAGetLastError());
		exit(1);
	}
	/* Connect to server. */
	if (connect(s, (struct sockaddr*) & sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("Attempt to connect to server failed: %d\n", WSAGetLastError());
		exit(1);
	}
	/* Return the file descriptor for the connected socket. */
	return(s);
}