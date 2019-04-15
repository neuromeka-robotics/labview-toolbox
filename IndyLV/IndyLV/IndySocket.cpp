
#include "stdafx.h"
#include "IndySocket.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_IP "192.168.10.108"
#define DEFAULT_PORT "6066"
#define DEFAULT_ROBOT_NAME "NRMK-Indy7"

#define NUM_CMD	10000

ConnectionInfo connection;
ACKPackage ackList[NUM_CMD];
bool isThreadRunning;
HANDLE  hThread;
DWORD   dwThreadID;

DWORD WINAPI SocketThread(LPVOID lpParam);

int ConnectIndy(char * ipaddr, int port, char * robotname)
{
	WSADATA wsaData;
	connection.ConnectSocket = INVALID_SOCKET;
	//SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *target = NULL, hints;
	int iResult;
	
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// resolve the server address and port
	//char ipaddr[] = DEFAULT_IP;
	//char port[] = DEFAULT_PORT;
	std::string str = std::to_string(port);
	char const *pchar = str.c_str();  //use char const* as target type
	iResult = getaddrinfo(ipaddr, pchar, &hints, &target);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return false;
	}

	// Create a SOCKET for connecting to server
	connection.ConnectSocket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	// Connect to server.
	iResult = connect(connection.ConnectSocket, target->ai_addr, (int)target->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(connection.ConnectSocket);
		connection.ConnectSocket = INVALID_SOCKET;
		return false;
	}

	freeaddrinfo(target);
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return false;
	}

	connection.invokeId = 0;

	isThreadRunning = true;
	hThread = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		SocketThread,       // thread function name
		NULL,          // argument to thread function 
		0,                      // use default creation flags 
		&dwThreadID);   // returns the thread identifier 

	if (hThread == NULL)
	{
		return false;
	}

	return true;
}

int DisconnectIndy()
{
	int iResult;

	// shutdown the connection since no more data will be sent
	iResult = shutdown(connection.ConnectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	// cleanup
	closesocket(connection.ConnectSocket);
	WSACleanup();

	isThreadRunning = false;
	WaitForSingleObject(hThread, 1000);
	CloseHandle(hThread);
	return true;
}

int CloseAll()
{
	isThreadRunning = false;
	DisconnectIndy();

	WaitForSingleObject(hThread, 1000);
	CloseHandle(hThread);

	return true;
}

int IsRobotReady(uint8_t * ready)
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	int iResult;
	char sendbuf[100];
	int cmdID = 31;
	int invokeID = connection.invokeId++;

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = invokeID;
	header.val.cmdId = cmdID;
	header.val.dataSize = 0;

	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	Sleep(200);

	*ready = (ackList[cmdID].data.boolVal) ? 1 : 0;

	return true;
}

int IsInMotion(uint8_t * in_motion)
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	int iResult;
	char sendbuf[100];
	int cmdID = 35;
	int invokeID = connection.invokeId++;

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = invokeID;
	header.val.cmdId = cmdID;
	header.val.dataSize = 0;

	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	Sleep(200);

	*in_motion = (ackList[cmdID].data.boolVal) ? 1 : 0;

	return true;
}

int MoveHome()
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");		
		return false;
	}

	HeaderCommand header;
	int iResult;

	//strcpy_s(header.val.robotName, strlen(DEFAULT_ROBOT_NAME), DEFAULT_ROBOT_NAME);
	//std::string name(DEFAULT_ROBOT_NAME);

	//memcpy(header.val.robotName, name.c_str(), name.length());
	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = connection.invokeId++;
	header.val.cmdId = 7;
	header.val.dataSize = 0;

	char sendbuf[SIZE_HEADER_COMMAND];
	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	return true;
}

int MoveZero()
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	int iResult;

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = connection.invokeId++;
	header.val.cmdId = 8;
	header.val.dataSize = 0;

	char sendbuf[SIZE_HEADER_COMMAND];
	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	return true;
}

int __cdecl SlowStop()
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	int iResult;

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = connection.invokeId++;
	header.val.cmdId = 5;
	header.val.dataSize = 0;

	char sendbuf[SIZE_HEADER_COMMAND];
	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	return true;
}

int GetJointsPosition(double *position, const int16_t num_joints)
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	int iResult;
	char sendbuf[100];
	int cmdID = 320;
	int invokeID = connection.invokeId++;

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = invokeID;
	header.val.cmdId = cmdID;
	header.val.dataSize = 0;

	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	Sleep(200);

	if (num_joints == 6)
	{
		for (int i = 0; i < 6; i++)
			position[i] = ackList[cmdID].data.double6dArr[i];
	}
	else if (num_joints == 7)
	{
		for (int i = 0; i < 7; i++)
			position[i] = ackList[cmdID].data.double7dArr[i];
	}
	else
	{
		for (int i = 0; i < num_joints; i++)
			position[i] = 0;
	}

	return true;
}

int GetJointsVelocity(double *velocity, const int16_t num_joints)
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	int iResult;
	char sendbuf[100];
	int cmdID = 321;
	int invokeID = connection.invokeId++;

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = invokeID;
	header.val.cmdId = cmdID;
	header.val.dataSize = 0;

	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	Sleep(200);

	if (num_joints == 6)
	{
		for (int i = 0; i < 6; i++)
			velocity[i] = ackList[cmdID].data.double6dArr[i];
	}
	else if (num_joints == 7)
	{
		for (int i = 0; i < 7; i++)
			velocity[i] = ackList[cmdID].data.double7dArr[i];
	}
	else
	{
		for (int i = 0; i < num_joints; i++)
			velocity[i] = 0;
	}

	return true;
}

int MoveJointTo(const double *target, const int16_t num_joints)
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	Data data;
	int iResult;
	char sendbuf[1024];

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = connection.invokeId++;
	header.val.cmdId = 9;
	header.val.dataSize = sizeof(double) * num_joints;

	memcpy(data.byte, target, header.val.dataSize);

	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}
	
	memcpy(sendbuf, &data, header.val.dataSize);

	iResult = send(connection.ConnectSocket, sendbuf, header.val.dataSize, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	return true;
}

int MoveJointBy(const double *target, const int16_t num_joints)
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	Data data;
	int iResult;
	char sendbuf[1024];

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = connection.invokeId++;
	header.val.cmdId = 10;
	header.val.dataSize = sizeof(double) * num_joints;

	memcpy(data.byte, target, header.val.dataSize);

	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	memcpy(sendbuf, &data, header.val.dataSize);

	iResult = send(connection.ConnectSocket, sendbuf, header.val.dataSize, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	return true;
}

int MoveTaskTo(const double *target)
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	Data data;
	int iResult;
	char sendbuf[1024];

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = connection.invokeId++;
	header.val.cmdId = 11;
	header.val.dataSize = sizeof(double) * 6;

	memcpy(data.byte, target, header.val.dataSize);

	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	memcpy(sendbuf, &data, header.val.dataSize);

	iResult = send(connection.ConnectSocket, sendbuf, header.val.dataSize, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	return true;
}

int MoveTaskBy(const double *target)
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	Data data;
	int iResult;
	char sendbuf[1024];

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = connection.invokeId++;
	header.val.cmdId = 12;
	header.val.dataSize = sizeof(double) * 6;

	memcpy(data.byte, target, header.val.dataSize);

	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	memcpy(sendbuf, &data, header.val.dataSize);

	iResult = send(connection.ConnectSocket, sendbuf, header.val.dataSize, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	return true;
}

int MoveWaypoints(const double *waypoints, const int16_t num_points, const int16_t num_joints)
{
	if (connection.ConnectSocket == INVALID_SOCKET) {
		printf("No connection found!\n");
		return false;
	}

	HeaderCommand header;
	Data data;
	int iResult;
	char sendbuf[1024];

	memset(header.val.robotName, 0, 20);
	memcpy(header.val.robotName, DEFAULT_ROBOT_NAME, strlen(DEFAULT_ROBOT_NAME));
	header.val.robotVersion[0] = '\0';
	header.val.stepInfo = 0x02;
	header.val.sof = 0x34;
	header.val.invokeId = connection.invokeId++;
	header.val.cmdId = 800;
	header.val.dataSize = 8;

	data.int2dArr[0] = 11; // Extended Joint Waypoint Set Command
	data.int2dArr[1] = num_points * (num_joints * sizeof(double)); // Extended Waypoint Set Command
	
	memcpy(sendbuf, header.byte, SIZE_HEADER_COMMAND);

	iResult = send(connection.ConnectSocket, sendbuf, SIZE_HEADER_COMMAND, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	memcpy(sendbuf, &data, header.val.dataSize);

	iResult = send(connection.ConnectSocket, sendbuf, header.val.dataSize, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	/* For Extended Data*/
	char * exDataBuff = new char[num_points*(num_joints * sizeof(double))];
	double * jTar = new double[num_joints];
	unsigned int exIdx = 0;

	for (int i = 0; i < num_points; i++)
	{
		for (int j = 0; j < num_joints; j++)
			jTar[j] = waypoints[(i * num_joints) + j];

		memcpy(exDataBuff + exIdx, jTar, num_joints * sizeof(double));
		exIdx += num_joints * sizeof(double);
	}

	iResult = send(connection.ConnectSocket, exDataBuff, num_points*(num_joints * sizeof(double)), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connection.ConnectSocket);
		WSACleanup();
		return false;
	}

	delete[] jTar;
	delete[] exDataBuff;

	return true;

}

DWORD WINAPI SocketThread(LPVOID lpParam)
{
	HeaderCommand resHeader;
	Data resData;

	char recvbuf[SIZE_DATA_MAX];
	int buffIdx;
	int iResult;
	bool dataIntegrity;

	while (isThreadRunning)
	{
		if (connection.ConnectSocket == INVALID_SOCKET) {
			printf("No connection found!\n");
			CloseAll();
			return false;
		}

		// Receive until the peer closes the connection
		dataIntegrity = true;
		buffIdx = 0;
		do {
			iResult = recv(connection.ConnectSocket, recvbuf + buffIdx, SIZE_HEADER_COMMAND - buffIdx, 0);
			if (iResult > 0)
			{
				printf("Bytes received: %d\n", iResult);
				buffIdx += iResult;

				if (buffIdx == SIZE_HEADER_COMMAND) break;
			}
			else if (iResult == 0)
			{
				printf("Connection closed\n");
				CloseAll();
				return false;
			}
			else
			{
				printf("recv failed with error: %d\n", WSAGetLastError());
				dataIntegrity = false;
			}
		} while (1);

		if (!dataIntegrity) continue;
		memcpy(resHeader.byte, recvbuf, SIZE_HEADER_COMMAND);

		if (resHeader.val.dataSize > 0)	//read data
		{
			buffIdx = 0;
			do
			{
				iResult = recv(connection.ConnectSocket, recvbuf + buffIdx, resHeader.val.dataSize - buffIdx, 0);
				if (iResult > 0)
				{
					printf("Bytes received: %d\n", iResult);
					buffIdx += iResult;

					if (buffIdx == resHeader.val.dataSize) break;
				}
				else if (iResult == 0)
				{
					printf("Connection closed\n");
					CloseAll();
					return false;
				}
				else
				{
					printf("recv failed with error: %d\n", WSAGetLastError());
					dataIntegrity = false;
				}
			} while (1);

			if (!dataIntegrity) continue;
			
			int ackID = resHeader.val.cmdId;
			memcpy(ackList[ackID].data.byte, recvbuf, resHeader.val.dataSize);
			ackList[ackID].invokeId = resHeader.val.invokeId;

		}
		else
		{
			int ackID = resHeader.val.cmdId;
			ackList[ackID].invokeId = resHeader.val.invokeId;
			memset(ackList[ackID].data.byte, 0, SIZE_DATA_MAX);
		}
		Sleep(100);
	}

	return 0;
}