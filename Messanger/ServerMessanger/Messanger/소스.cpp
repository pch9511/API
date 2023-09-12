#pragma warning(disable:4996)
#include <windows.h>
#include <stdio.h>
#define ID_EDIT 501 //����Ʈ�ڽ��� ID
#define ID_LIST 601 //����Ʈ�ڽ��� ID

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("ServerMessanger");

HWND g_hEdit; //����Ʈ�ڽ��� �ڵ�
HWND g_hList; //����Ʈ�ڽ��� �ڵ�
const unsigned short g_uPort = 7878;
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
		0, 0, 350, 240,
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
	
	for (;;)
	{
		clientsock = accept(*P, (sockaddr *)&addr_client, &addrlen_clt);

		if (clientsock == INVALID_SOCKET)
		{
			continue;
		}
		else 
		{
			SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)TEXT("Ŭ���̾�Ʈ�� �����Ͽ����ϴ�."));
		}

		nReturn = recv(clientsock, buf, buflen, 0);
		if (nReturn == 0 || nReturn == SOCKET_ERROR) 
		{
			sprintf(strTemp, TEXT("���� ���� : %s"), buf);
			SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)strTemp);
		}
		else
		{
			sprintf(strTemp, TEXT("������ �޼��� : %s"), buf);
			SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)strTemp);
		}
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	char strEdit[128]; //����Ʈ �ڽ� ����
	char strList[128]; //����Ʈ �ڽ� �� �޺� �ڽ� ���� ����
	DWORD ThreadID;
	static HANDLE hThread;
	char strMes[128] = TEXT("���� �����Դϴ�.");

	switch (iMessage) 
	{
	case WM_CREATE:
		nReturn = WSAStartup(WORD(2.0), &wsadata);

		listensock = socket(AF_INET, SOCK_STREAM, 0);
		
		addr_server.sin_family = AF_INET;
		addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
		addr_server.sin_port = htons(g_uPort);
		nReturn = bind(listensock, (sockaddr *)&addr_server, sizeof(sockaddr));

		nReturn = listen(listensock, 1);

		hThread = CreateThread(NULL, 0, ThreadFunc, &listensock, 0, &ThreadID);
		CreateWindow(TEXT("button"), TEXT("������"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			230, 140, 80, 25, hWnd, (HMENU)200, g_hInst, NULL);			//������ ��ư
		g_hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE,
			20, 110, 300, 20, hWnd, (HMENU)ID_EDIT, g_hInst, NULL);		//����Ʈ ������ ����
		g_hList = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
			20, 40, 300, 80, hWnd, (HMENU)ID_LIST, g_hInst, NULL);		//����Ʈ �ڽ� ����
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 200:	//�Ϲ� ��ư �̺�Ʈ
			GetWindowText(g_hEdit, strEdit, 128);
			nReturn = send(clientsock, strEdit, sizeof(strEdit), 0);
			if (nReturn)
			{
				sprintf(buf, TEXT("�۽��� �޼��� : %s"), strEdit);
				SendMessage(g_hList, LB_ADDSTRING, 0, (LPARAM)buf);
			}
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 20, 20, strMes, strlen(strMes));
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