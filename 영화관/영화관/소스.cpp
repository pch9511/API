#include<windows.h>
#include<stdlib.h>
#include<stdio.h>
#include"resource.h"
#include<time.h>
#define MAX_COUNT 100
#define SEAT_MAX 5
#define TICKET_MAX 6
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
HANDLE hSem_TheaterIn; //영화관 입장
HANDLE hSem_Wait;// 의자 앉아서 대기 5명
HANDLE hSem_Ticketing; //티켓팅 6명
HANDLE hSem_Toilet; //화장실 대기열 1명                       A , B ,Toilet 3군데는 랜덤으로 들어와야함
HANDLE hSem_Movie1; //1번 영화 ex. 20명						A 티켓을 구매했다면 A상영관으로 이동 상영중이라면 대기 
HANDLE hSem_Movie2; //2번 영화 ex. 10명						B도 위와 동일 
HANDLE hSem_Exit;	//영화관 출구 나가기 
HANDLE hSem_Exit2;	// 2번 출구 나가기
HANDLE hSem_Count; //빈자리 찾기 
HANDLE hSem_Count2;
int Rx = 350, Ry = 450, Count = 0, Count2 = 0;
int Total=0; // 총 수익
int a = 0; //몇번째 사람인가?
struct Customer {			//고객의 위치에 따라 움직임 제어 , 고객이 산 티켓 값 
	RECT rt;			//자기 자신을 초기화 시키기 
	POINT pt;         //움직임 좌표 제어 
	int TicketNum;
	int LocNumCopy;
	BOOL Sit;
	BOOL Line;
	BOOL Toilet;
	BOOL Movie;
	BOOL Movie2;
	//RECT or POINT 제어가 더 필요하면 추가 
};
struct Seat {
	POINT pt;
	int Sit;
	int arrive;
};
struct TicketLine {
	POINT pt;
	int Line;
	int arrive;
};
struct Toilet {
	POINT pt;
	int arrive;
};
Customer Ct[MAX_COUNT];
Seat St[SEAT_MAX];
TicketLine Tl[TICKET_MAX];
Toilet WC;
POINT M1, M2;
LPCTSTR lpszClass = TEXT("Theater");
DWORD WINAPI Thread(LPVOID);
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstanceMOVEPOINT
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = CreatePatternBrush((HBITMAP)LoadImage(0, TEXT("프로토타입.bmp"), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1200, 700,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	HANDLE hThread[MAX_COUNT];
	DWORD ThreadID[MAX_COUNT];
	int q[MAX_COUNT];
	int i, j = 0, k, cnt = 0, cnt2 = 0;
	TCHAR str[256];
	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;
		hSem_TheaterIn = CreateSemaphore(NULL, 12, 12, NULL);
		hSem_Wait = CreateSemaphore(NULL, 5, 5, NULL);
		hSem_Ticketing = CreateSemaphore(NULL, 6, 6, NULL);
		hSem_Toilet = CreateSemaphore(NULL, 1, 1, NULL);
		hSem_Count = CreateSemaphore(NULL, 1, 1, NULL);
		hSem_Count2 = CreateSemaphore(NULL, 1, 1, NULL);
		hSem_Movie1 = CreateSemaphore(NULL, 20, 20, NULL);
		hSem_Movie2 = CreateSemaphore(NULL, 10, 10, NULL);
		hSem_Exit = CreateSemaphore(NULL, 1, 1, NULL);
		hSem_Exit2 = CreateSemaphore(NULL, 1, 1, NULL);
		for (i = 0; i < MAX_COUNT; i++) {
			k = rand() % 8;
			if (k <= 1) {
				Ct[i].Sit = TRUE;
				Ct[i].Toilet = FALSE;
			}
			else if (k == 2) {
				Ct[i].Toilet = TRUE;
				Ct[i].Sit = FALSE;
			}
			else if(k ==3){
				Ct[i].Toilet = TRUE;
				Ct[i].Sit = TRUE;
			}
			else {
				Ct[i].Toilet = FALSE;
				Ct[i].Sit = FALSE;
			}
			Ct[i].Line = TRUE;
			Ct[i].pt.x = 230;
			Ct[i].pt.y = 610;
			q[i] = i;
			hThread[i] = CreateThread(NULL, 0, Thread, &q[i], 0, &ThreadID[i]);
			CloseHandle(hThread[i]);
		}
		for (i = 0; i < SEAT_MAX; i++) {
			St[i].pt.y = 520;
			St[i].pt.x = 400 + (i * 60);
			St[i].Sit = 0;
			St[i].arrive = 0;
		}
		for (i = 0; i < TICKET_MAX; i++) {
			if (i >= 3) {
				Tl[i].pt.y = 240;
			}
			else if (i<3) {
				Tl[i].pt.y = 90;
			}
			if (i == 3) {
				j = 0;
			}
			Tl[i].arrive = 0;
			Tl[i].Line = 0;
			Tl[i].pt.x = 230 + (j * 30);
			j++;
		}
		M1.x = 650;
		M1.y = 150;
		M2.x = 650;
		M2.y = 320;
		WC.pt.x = 450;
		WC.pt.y = 100;
		WC.arrive = 0;
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		wsprintf(str, TEXT("총 수익 : %d"), Total);
		TextOut(hdc, 1000, 600, str, lstrlen(str));
		for (i = 0; i < SEAT_MAX; i++) {
			St[i].pt.y = 520;
			St[i].pt.x = 400 + (i * 60);
			if (St[i].arrive == 1) {
				TextOut(hdc, 400 + (i * 60), 520, TEXT("O"), 1);
			}
			else {
				TextOut(hdc, 400 + (i * 60), 520, TEXT("X"), 1);
			}
		}
		for (i = 0; i < TICKET_MAX; i++) {
			if (Tl[i].arrive == 1) {
				if (i >= 3) {
					cnt2++;
				}
				else {
					cnt++;
				}
			}
			wsprintf(str, TEXT("%d/3"), cnt);
			TextOut(hdc, 130, 85, str, lstrlen(str));
			wsprintf(str, TEXT("%d/3"), cnt2);
			TextOut(hdc, 130, 240, str, lstrlen(str));
		}
		if (Count < 20) {
			wsprintf(str, TEXT("%d/20"), Count);
			TextOut(hdc, 705, 120, str, lstrlen(str));
		}
		else {
			TextOut(hdc, 705, 120, TEXT("상영중"), 3);
		}
		if (Count2 < 10) {
			wsprintf(str, TEXT("%d/10"), Count2);
			TextOut(hdc, 710, 305, str, lstrlen(str));
		}
		else {
			TextOut(hdc, 710, 305, TEXT("상영중"), 3);
		}
		if (WC.arrive == 1) {
			TextOut(hdc, 445, 60, TEXT("O"),1);
		}
		else {
			TextOut(hdc, 445, 60, TEXT("X"), 1);
		}
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		CloseHandle(hSem_Wait);
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
void MoveToX(int x, int arrive, int a) {
	TCHAR str[256];
	HDC hdc;

	hdc = GetDC(hWndMain);
	while (arrive != Ct[a].pt.x) {
		Ct[a].rt.left = Ct[a].pt.x;
		Ct[a].rt.top = Ct[a].pt.y;
		Ct[a].rt.right = Ct[a].pt.x + 30;
		Ct[a].rt.bottom = Ct[a].pt.y + 30;
		if (arrive > x) {
			Ct[a].pt.x += 1;
		}
		else {
			Ct[a].pt.x -= 1;
		}
		TextOut(hdc, Ct[a].pt.x, Ct[a].pt.y, TEXT("I"), 1);
		Sleep(10);
	}
	ReleaseDC(hWndMain, hdc);
	InvalidateRect(hWndMain, NULL, TRUE);
}
void MoveToY(int y, int arrive, int a) {
	TCHAR str[256];
	HDC hdc;

	hdc = GetDC(hWndMain);
	while (arrive != Ct[a].pt.y) {
		Ct[a].rt.left = Ct[a].pt.x;
		Ct[a].rt.top = Ct[a].pt.y;
		Ct[a].rt.right = Ct[a].pt.x + 30;
		Ct[a].rt.bottom = Ct[a].pt.y + 30;
		if (arrive > y) {
			Ct[a].pt.y += 1;
		}
		else {
			Ct[a].pt.y -= 1;
		}
		wsprintf(str, TEXT("%d"), a);
		TextOut(hdc, Ct[a].pt.x, Ct[a].pt.y, TEXT("I"), 1);
		Sleep(10);
	}
	ReleaseDC(hWndMain, hdc);
	InvalidateRect(hWndMain, NULL, TRUE);
}
DWORD WINAPI Thread(LPVOID Param)
{
	int *b = (int*)Param;
	int a = *b;  // 함수 번호 가져오는 부분
	HDC hdc;
	int i;
	RECT RCT;
	int t = -1;
	int test = 0;
	TCHAR str[256];

	srand((unsigned int)time(NULL));
	WaitForSingleObject(hSem_TheaterIn, INFINITE);
	Sleep(a * 100);
	if (Ct[a].Sit == TRUE) {
		WaitForSingleObject(hSem_Wait, INFINITE);
		MoveToY(Ct[a].pt.y, Ry, a);
		WaitForSingleObject(hSem_Count, INFINITE);
		while (1) {
			srand(GetTickCount());
			Sleep(a * 20);
			Ct[a].LocNumCopy = rand() % 5;
			if (St[Ct[a].LocNumCopy].Sit == 0) {
				St[Ct[a].LocNumCopy].Sit = 1;
				break;
			}
		}
		ReleaseSemaphore(hSem_Count, 1, NULL);
		MoveToX(Ct[a].pt.x, St[Ct[a].LocNumCopy].pt.x, a);
		MoveToY(Ct[a].pt.y, St[Ct[a].LocNumCopy].pt.y, a);
		St[Ct[a].LocNumCopy].arrive = 1;
		Sleep(2000);
	}
	if (Ct[a].Toilet == TRUE) {
		WaitForSingleObject(hSem_Toilet, INFINITE);
		if (Ct[a].Sit == TRUE) {
			Sleep(1000);
			ReleaseSemaphore(hSem_Wait, 1, NULL);
			St[Ct[a].LocNumCopy].Sit = 0;
			St[Ct[a].LocNumCopy].arrive = 0;
		}
		MoveToY(Ct[a].pt.y, Ry, a);
		if (Ct[a].Sit == TRUE) {
			MoveToY(Ct[a].pt.y, Ry, a);
			MoveToX(Ct[a].pt.x, WC.pt.x, a);
			MoveToY(Ct[a].pt.y, WC.pt.y, a);
		}
		else {
			MoveToX(Ct[a].pt.x, WC.pt.x, a);
			MoveToY(Ct[a].pt.y, WC.pt.y, a);
		}
		WC.arrive = 1;
		Sleep(2000);
	}
	test = Ct[a].LocNumCopy;
	WaitForSingleObject(hSem_Ticketing, INFINITE);
	Sleep(1000);
	WaitForSingleObject(hSem_Count2, INFINITE);	
	if (Ct[a].Sit == TRUE) {
		St[Ct[a].LocNumCopy].arrive = 0;
	}
	while (1) {
		srand(GetTickCount());
		Ct[a].LocNumCopy = rand() % 6;
		if (Tl[Ct[a].LocNumCopy].Line == 0) {
			Tl[Ct[a].LocNumCopy].Line = 1;
			//wsprintf(str, TEXT("%d"), Ct[a].LocNumCopy);
			//MessageBox(hWndMain, str, TEXT("TEST"), MB_OK);
			break;
		}
	}
	if (Ct[a].Sit == TRUE) {
		ReleaseSemaphore(hSem_Wait, 1, NULL);
		St[test].Sit = 0;
	}
	ReleaseSemaphore(hSem_Count2, 1, NULL);
	if (Ct[a].Toilet == TRUE) {
		ReleaseSemaphore(hSem_Toilet, 1, NULL);
		WC.arrive = 0;
		MoveToY(Ct[a].pt.y, Ry - 280, a);
		MoveToX(Ct[a].pt.x, Rx, a);
	}
	else {
		MoveToY(Ct[a].pt.y, Ry, a);
		MoveToX(Ct[a].pt.x, Rx, a);
	}
	InvalidateRect(hWndMain, NULL, TRUE);
	MoveToY(Ct[a].pt.y, Tl[Ct[a].LocNumCopy].pt.y, a);
	MoveToX(Ct[a].pt.x, Tl[Ct[a].LocNumCopy].pt.x, a);
	Ct[a].TicketNum = rand() % 2;
	Tl[Ct[a].LocNumCopy].arrive = 1;
	Sleep(2000);
	Tl[Ct[a].LocNumCopy].arrive = 0;
	ReleaseSemaphore(hSem_TheaterIn, 1, NULL);
	ReleaseSemaphore(hSem_Ticketing, 1, NULL);
	Tl[Ct[a].LocNumCopy].Line = 0;
	if (Ct[a].TicketNum == 0) {
		Total += 2000;
		MoveToX(Ct[a].pt.x, Rx, a);
		MoveToY(Ct[a].pt.y, M1.y, a);
		//wsprintf(str, TEXT("빠진 티켓자리 : %d  의 상태 %d"), Ct[a].LocNumCopy, Tl[Ct[a].LocNumCopy].Line);
		//MessageBox(hWndMain, str, TEXT("TEST"), MB_OK);
		WaitForSingleObject(hSem_Movie1, INFINITE);
		MoveToX(Ct[a].pt.x, M1.x, a);
		Count++;
		if (Count == 20) {
			Ct[a].pt.x = 990;
			Ct[a].pt.y = 90;
			
			Sleep(4000);
			Count = 0;
			ReleaseSemaphore(hSem_Movie1, 1, NULL);
			WaitForSingleObject(hSem_Exit, INFINITE);	
			Sleep(a * 1000);
			MoveToX(Ct[a].pt.x, 1050, a);
			ReleaseSemaphore(hSem_Exit, 1, NULL);
			MoveToY(Ct[a].pt.y, 520, a);
			MoveToX(Ct[a].pt.x, 1090, a);
		}
	}
	else if (Ct[a].TicketNum == 1) {
		Total += 1000;
		MoveToX(Ct[a].pt.x, Rx, a);
		MoveToY(Ct[a].pt.y, M2.y, a);
	//	wsprintf(str, TEXT("빠진 티켓자리 : %d  의 상태 %d"), Ct[a].LocNumCopy, Tl[Ct[a].LocNumCopy].Line);
	//	MessageBox(hWndMain, str, TEXT("TEST"), MB_OK);
		WaitForSingleObject(hSem_Movie2, INFINITE);
		MoveToX(Ct[a].pt.x, M2.x, a);
		Count2++;
		if (Count2 == 10) {
			
			Ct[a].pt.x = 990;
			Ct[a].pt.y = 280;
			Sleep(4000);
			Count2 = 0;
			ReleaseSemaphore(hSem_Movie2, 1, NULL);
			WaitForSingleObject(hSem_Exit2, INFINITE);
			Sleep(a * 1000);
			MoveToX(Ct[a].pt.x, 1050, a);
			ReleaseSemaphore(hSem_Exit2, 1, NULL);
			MoveToY(Ct[a].pt.y, 520, a);
			MoveToX(Ct[a].pt.x, 1090, a);
		}
	}
	return 0;
}