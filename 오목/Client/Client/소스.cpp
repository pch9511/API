#pragma warning(disable:4996)
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#define ID_EDIT 501		//에디트박스 ID
#define ID_LIST 601		//리스트박스 ID

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("Client");
HWND g_hEdit;	//에디트박스 핸들
HWND g_hEdit2;
HWND hWndMain;

const char g_szlpAddress[17] = TEXT("127.0.0.1");
//const unsigned short g_uPort = 7878;

int nReturn;
WSADATA wsadata;
SOCKET clientsock;
sockaddr_in addr_client;
int addrlen_clt = sizeof(sockaddr);
const int buflen = 4096;
char buf[buflen];
char strTemp[buflen];
char PointX[20], PointY[20];
char strSize[20];

int LineX[8] = { 100,150,200,250,300,350,400,450 };
int LineY[8] = { 100,150,200,250,300,350,400,450 };
int Size;
int Px, Py;

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
	WndClass.lpszMenuName = NULL;
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
	HBRUSH hBrush, hOldBrush;
	SOCKET *P = (SOCKET *)Param;
	int i, j = 0, cnt = 0;


	for (;;)
	{
		nReturn = recv(clientsock, buf, buflen, 0);		//서버에서부터 받아온 정보를 받음
		MessageBox(hWndMain, buf, TEXT("TEST"), MB_OK);
		if (nReturn == 0 || nReturn == SOCKET_ERROR)
		{

		}
		else
		{
			MessageBox(hWndMain, TEXT("받음"), TEXT("TEST"), MB_OK); 
			for (i = 0; i < buflen; i++)
			{
				if (buf[i] == '|')
				{
					j = 0;
					cnt++;
				}
				else if (cnt == 0)
				{
					strSize[j] = buf[i];
					j++;
				}
				else if (cnt == 1)
				{
					PointX[j] = buf[i];
					j++;
				}
				else
				{
					PointY[j] = buf[i];
					j++;
				}
			}
			MessageBox(hWndMain, PointX, TEXT("TEST"), MB_OK);
			MessageBox(hWndMain, PointY, TEXT("TEST"), MB_OK);
			Px = atoi(PointX);
			Py = atoi(PointY);
			Size = atoi(strSize);
			hdc = GetDC(hWndMain);
			if (Px != 0) {
				Ellipse(hdc, Px - 20, Py - 20, Px + 20, Py + 20);
			}
			ReleaseDC(hWndMain, hdc);
			InvalidateRect(hWndMain, NULL, FALSE);
			cnt = 0;
		}
	}
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd,
	UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	char strEdit[128];	//에디트 박스 변수
	DWORD ThreadID;
	static HANDLE hThread, hThread2;
	POINT pt;
	int i, j;

	switch (iMessage)
	{
	case WM_CREATE:
		hWndMain = hWnd;
		CreateWindow(TEXT("static"), TEXT("Port"), WS_CHILD | WS_VISIBLE,
			20, 20, 100, 20, hWnd, (HMENU)-1, g_hInst, NULL);
		g_hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE,
			50, 20, 100, 20, hWnd, (HMENU)ID_EDIT, g_hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("닉네임"), WS_CHILD | WS_VISIBLE,
			200, 20, 100, 20, hWnd, (HMENU)-1, g_hInst, NULL);
		g_hEdit2 = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE,
			250, 20, 100, 20, hWnd, (HMENU)ID_EDIT, g_hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("접속"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			500, 20, 100, 30, hWnd, (HMENU)100, g_hInst, NULL);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 100:
			GetWindowText(g_hEdit, strEdit, 128);
			i = atoi(strEdit);
			nReturn = WSAStartup(WORD(2.0), &wsadata);
			clientsock = socket(AF_INET, SOCK_STREAM, 0);

			addr_client.sin_family = AF_INET;
			addr_client.sin_addr.s_addr = inet_addr(g_szlpAddress);
			addr_client.sin_port = htons(i);
			nReturn = connect(clientsock, (sockaddr *)&addr_client, addrlen_clt);
			if (!nReturn)
				MessageBox(hWnd, TEXT("서버연결"), TEXT("TEST"), MB_OK);
			hThread = CreateThread(NULL, 1000, ThreadFunc, &clientsock, 0, &ThreadID);
			GetWindowText(g_hEdit2, strEdit, 128);
			lstrcat(strTemp, strEdit);
			lstrcat(strTemp, TEXT("|"));
			nReturn = send(clientsock, strTemp, sizeof(strTemp), 0);
			if (nReturn)
				MessageBox(hWnd, TEXT("닉네임 전송"), TEXT("TEST"), MB_OK);
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_LBUTTONDOWN:
		GetCursorPos(&pt);
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
						wsprintf(strEdit, TEXT("%d"), LineX[i]);
						lstrcat(strTemp, strEdit);
						lstrcat(strTemp, TEXT("|"));
						wsprintf(strEdit, TEXT("%d"), LineY[j]);
						lstrcat(strTemp, strEdit);
						MessageBox(hWndMain, strTemp, TEXT("TEST"), MB_OK);
						nReturn = send(clientsock, strTemp, sizeof(strTemp), 0);
						if (nReturn)
							MessageBox(hWnd, TEXT("오목 두는곳 놓음"), TEXT("TEST"), MB_OK);
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
