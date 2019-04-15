#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

struct ConnectionInfo
{
	SOCKET ConnectSocket;
	char robotName[20];	
	int invokeId;	
};
extern ConnectionInfo connection;

enum
{
	SIZE_HEADER = 52,
	SIZE_COMMAND = 4,
	SIZE_HEADER_COMMAND = 56,
	SIZE_DATA_MAX = 200,
	SIZE_DATA_ASCII_MAX = 32
};

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
struct HeaderCommandStruct
{
	char robotName[20];
	char robotVersion[12];
	unsigned char stepInfo;
	unsigned char sof;		//source of Frame
	int invokeId;
	int dataSize;
	char reserved[10];
	int cmdId;
};
#pragma pack(pop)   /* restore original alignment from stack */
union HeaderCommand
{
	unsigned char byte[SIZE_HEADER_COMMAND];
	HeaderCommandStruct val;
};

union Data
{
	unsigned char byte[SIZE_DATA_MAX];
	char asciiStr[SIZE_DATA_ASCII_MAX + 1];
	char str[200];
	char charVal;
	bool boolVal;
	short shortVal;
	int intVal;
	float floatVal;
	double doubleVal;
	bool boolArr[200];
	char char2dArr[2];
	char char3dArr[3];
	char char6dArr[6];
	char char7dArr[200];
	char charArr[200];
	int int2dArr[2];
	int int3dArr[3];
	int int6dArr[6];
	int int7dArr[6];
	int intArr[50];
	float float3dArr[3];
	float float6dArr[6];
	float float7dArr[7];
	float floatArr[50];
	double double3dArr[3];
	double double6dArr[6];
	double double7dArr[6];
	double doubleArr[25];
};

struct ACKPackage
{
	int invokeId;
	Data data;
};

//Function declarations
extern "C" __declspec(dllexport) int __cdecl ConnectIndy(char * ipaddr, int port, char * robotname);
extern "C" __declspec(dllexport) int __cdecl DisconnectIndy();
extern "C" __declspec(dllexport) int __cdecl CloseAll();
extern "C" __declspec(dllexport) int __cdecl IsRobotReady(uint8_t * ready);
extern "C" __declspec(dllexport) int __cdecl IsInMotion(uint8_t * in_motion);
extern "C" __declspec(dllexport) int __cdecl MoveHome();
extern "C" __declspec(dllexport) int __cdecl MoveZero();
extern "C" __declspec(dllexport) int __cdecl SlowStop();
extern "C" __declspec(dllexport) int __cdecl MoveJointTo(const double *target, const int16_t num_joints);
extern "C" __declspec(dllexport) int __cdecl MoveJointBy(const double *target, const int16_t num_joints);
extern "C" __declspec(dllexport) int __cdecl MoveTaskTo(const double *target);
extern "C" __declspec(dllexport) int __cdecl MoveTaskBy(const double *target);
extern "C" __declspec(dllexport) int __cdecl GetJointsPosition(double *position, const int16_t num_joints);
extern "C" __declspec(dllexport) int __cdecl GetJointsVelocity(double *velocity, const int16_t num_joints);
extern "C" __declspec(dllexport) int __cdecl MoveWaypoints(const double *waypoints, const int16_t num_points, const int16_t num_joints);
