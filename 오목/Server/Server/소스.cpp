#pragma warning(disable:4996)
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#define ID_EDIT 501 //에디트박스의 ID
#define ID_LIST 601 //리스트박스의 ID

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("Server");

HWND g_hButton; //버튼박스의 핸들
HWND g_hEdit; //에디트박스의 핸들
HWND hWndMain;

int nReturn;
WSADATA wsadata;
SOCKET listensock;
SOCKET clientsock = 0;
sockaddr_in addr_server;
sockaddr_in addr_client;
int addrlen_srv = sizeof(sockaddr);
int addrlen_clt = sizeof(sockaddr);
in_addr in;
hostent *ht;
const int buflen = 4096;
char buf[buflen];
char strTemp[buflen];
char Nick[20];
char PointX[20];
char PointY[20];

int LineX[8] = { 100,150,200,250,300,350,400,450 };
int LineY[8] = { 100,150,200,250,300,350,400,450 };
int Size = 0;
int Px, Py;
char strEdit[128], strEdit2[128]; //에디트 박스 변수

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

DWORD WINAPI ThreadFunc(LPVOID Param)
{
	HDC hdc;
	BYTE Red = 0;
	BYTE Green = 0;
	BYTE Blue = 0;
	SOCKET *P = (SOCKET *)Param;
	int i, j = 0, cnt = 0;

	for (;;)
	{
		clientsock = accept(*P, (sockaddr *)&addr_client, &addrlen_clt);
		if (clientsock == INVALID_SOCKET)
		{
		}
		else
		{
			MessageBox(hWndMain, TEXT("클라이언트 접속"), TEXT("TEST"), MB_OK);
		}

		nReturn = recv(clientsock, buf, buflen, 0);

		if (nReturn == 0 || nReturn == SOCKET_ERROR)
		{
		}
		else
		{

		MessageBox(hWndMain, TEXT("IN?"), TEXT("TEST"), MB_OK);
			for (i = 0; i < buflen; i++)
			{
				if (buf[i] == '|')
				{
					j = 0;
					cnt++; 
				}
				else if(cnt==0)
				{
					Nick[j] = buf[i];
					j++;
				}
				else if (cnt == 1)
				{
					PointX[j] = buf[i];
					j++;
				}
				else {
					PointY[j] = buf[i];
					j++;
				}
			}
			MessageBox(hWndMain, Nick, TEXT("TEST"), MB_OK);
			Px = atoi(PointX);
			Py = atoi(PointY);
			hdc = GetDC(hWndMain);
			if (Px != 0)
			{
				Ellipse(hdc, Px - 20, Py - 20, Px + 20, Py + 20);
			}
			ReleaseDC(hWndMain, hdc);
			InvalidateRect(hWndMain, NULL, FALSE);
			ShowWindow(g_hButton, SW_SHOW);
		}
	}
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	DWORD ThreadID;
	static HANDLE hThread;
	POINT pt;
	int i, j;
	char test[128];
	switch (iMessage)
	{
	case WM_CREATE:
		hWndMain = hWnd;
		g_hButton = CreateWindow(TEXT("button"), TEXT("시작"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			150, 500, 80, 25, hWnd, (HMENU)100, g_hInst, NULL);
		ShowWindow(g_hButton, SW_HIDE);
		CreateWindow(TEXT("button"), TEXT("시작"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			500, 140, 80, 25, hWnd, (HMENU)200, g_hInst, NULL);			//보내기 버튼
		g_hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE,
			500, 70, 80, 25, hWnd, (HMENU)ID_EDIT, g_hInst, NULL);		//에디트 윈도우 생성
		CreateWindow(TEXT("static"), TEXT("Port"), WS_CHILD | WS_VISIBLE,
			470, 70, 30, 25, hWnd, (HMENU)-1, g_hInst, NULL);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 200:	//일반 버튼 이벤트
			GetWindowText(g_hEdit, strEdit, 128);
			i = atoi(strEdit);
			nReturn = WSAStartup(WORD(2.0), &wsadata);

			listensock = socket(AF_INET, SOCK_STREAM, 0);

			addr_server.sin_family = AF_INET;
			addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
			addr_server.sin_port = htons(i);
			nReturn = bind(listensock, (sockaddr *)&addr_server, sizeof(sockaddr));

			nReturn = listen(listensock, 1);
			if (!nReturn)
				MessageBox(hWnd, strEdit, TEXT("TEST"), MB_OK);
			hThread = CreateThread(NULL, 1000, ThreadFunc, &listensock, 0, &ThreadID);
			break;
		case 100:
			if (Size != 0) {
				wsprintf(strEdit, TEXT("%d"), Size);
				lstrcat(strTemp, strEdit);
				nReturn = send(clientsock, strTemp, sizeof(strTemp), 0);
			}
			break;
		case ID_SIZE5:
			Size = 5;
			break;
		case ID_SIZE7:
			Size = 7;
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_LBUTTONDOWN:
		GetCursorPos(&pt);				//마우스 클릭 좌표를 받음
		ScreenToClient(hWnd, &pt);
		for (i = 0; i < Size; i++)
		{
			if (pt.x > LineX[i] - 20 && pt.x<LineX[i] + 20)
			{
				for (j = 0; j < Size; j++)
				{
					if (pt.y > LineY[j] - 20 && pt.y < LineY[j] + 20)
					{
						hdc = GetDC(hWnd);
						Ellipse(hdc, LineX[i] - 20, LineY[j] - 20, LineX[i] + 20, LineY[j] + 20); 
						ReleaseDC(hWnd, hdc);
						lstrcat(strTemp, TEXT("|"));
						wsprintf(strEdit2, TEXT("%d"), LineX[i]);
						lstrcat(strTemp, strEdit2);
						lstrcat(strTemp, TEXT("|"));
						wsprintf(strEdit2, TEXT("%d"), LineY[j]);
						lstrcat(strTemp, strEdit2);
						MessageBox(hWnd, strTemp, TEXT("TEST"), MB_OK);
						nReturn = send(clientsock, strTemp, sizeof(strTemp), 0);	//클라에게 정보를 줌 
						if (nReturn)
							MessageBox(hWnd, TEXT("오목 두는곳 놓음"), TEXT("TEST"), MB_OK);
						lstrcpy(strTemp, strEdit);
					}
				}
			}
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		Rectangle(hdc, LineX[0], LineY[0], LineX[Size], LineY[Size]);
		for (i = 0; i < Size; i++)
		{
			MoveToEx(hdc, LineX[i], LineY[0], NULL);
			LineTo(hdc, LineX[i], LineY[Size]);
			MoveToEx(hdc, LineX[0], LineY[i], NULL);
			LineTo(hdc, LineX[Size], LineY[i]);
		}
		TextOut(hdc, 20, 500, Nick, lstrlen(Nick));
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		closesocket(clientsock);
		WSACleanup();
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}