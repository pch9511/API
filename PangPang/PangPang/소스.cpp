#include <windows.h>
#include <math.h>
#include "resource.h"
#define MAXBALL 1000   //���� �ִ� ����
#define MAXBULLET 3
const double A = 0.03; //�߷� ���ӵ�
const double G = 0.05; //�߷�
const double AR = 0.003; //������ ����
struct tag_Ball
{
	BOOL Exist;  //���� ���� ����
	double x, y; //���� ��ġ
	int r; //������
	double dx, dy;//��Ÿ��
	COLORREF col; //����
};
struct tag_Bullet
{
	BOOL Exist;
	int x, y;
};
tag_Bullet arBullet[MAXBULLET];
int cx;
int Px,Py,PlayerPos;
int nStage=1,Life,Score;
const int cy = 360;
TCHAR str[256];
tag_Ball arBall[MAXBALL];
HBITMAP hBit,hPlayer,hBullet;
HWND hWndMain;
RECT crt;
BOOL bAuto, END = FALSE, TIMERCON = FALSE;

int HitCheck();
void GameEnd();
void SplitBall(int t);
void StartStage(BOOL Start);
void NewBullet(BOOL Shoot);
void MoveBullet();
void MoveBall();
void NewBall(BOOL bLeft);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("Pang Pang");
void TransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask)
{
	BITMAP bm;
	COLORREF cColor;
	HBITMAP bmAndBack, bmAndObject, bmAndMem, bmSave;
	HBITMAP bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
	HDC		hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
	POINT	ptSize;

	hdcTemp = CreateCompatibleDC(hdc);
	SelectObject(hdcTemp, hbitmap);
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;
	DPtoLP(hdcTemp, &ptSize, 1);

	hdcBack = CreateCompatibleDC(hdc);
	hdcObject = CreateCompatibleDC(hdc);
	hdcMem = CreateCompatibleDC(hdc);
	hdcSave = CreateCompatibleDC(hdc);

	bmAndBack = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
	bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
	bmAndMem = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
	bmSave = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

	bmBackOld = (HBITMAP)SelectObject(hdcBack, bmAndBack);
	bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
	bmMemOld = (HBITMAP)SelectObject(hdcMem, bmAndMem);
	bmSaveOld = (HBITMAP)SelectObject(hdcSave, bmSave);

	SetMapMode(hdcTemp, GetMapMode(hdc));

	BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

	cColor = SetBkColor(hdcTemp, clrMask);

	BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

	SetBkColor(hdcTemp, cColor);

	BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, x, y, SRCCOPY);
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);
	BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);
	BitBlt(hdc, x, y, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY);
	BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(hdcBack, bmBackOld));
	DeleteObject(SelectObject(hdcObject, bmObjectOld));
	DeleteObject(SelectObject(hdcMem, bmMemOld));
	DeleteObject(SelectObject(hdcSave, bmSaveOld));

	DeleteDC(hdcMem);
	DeleteDC(hdcBack);
	DeleteDC(hdcObject);
	DeleteDC(hdcSave);
	DeleteDC(hdcTemp);
}


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

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hbit)
{
	HDC MemoryDC; //�޸� ��� 
	int BitmapX, BitmapY;
	BITMAP Bitmap_;
	HBITMAP OldBitmap;
	MemoryDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemoryDC, hbit);
	GetObject(hbit, sizeof(BITMAP), &Bitmap_);
	BitmapX = Bitmap_.bmWidth;
	BitmapY = Bitmap_.bmHeight;
	BitBlt(hdc, x, y, BitmapX, BitmapY, MemoryDC, 0, 0, SRCCOPY);
	SelectObject(MemoryDC, OldBitmap);
	DeleteDC(MemoryDC);
}
void OnTimer()
{
	HDC hdc;
	HDC hMemDC;
	HBITMAP OldBit;
	HBRUSH hBrush, hOldBrush;
	HPEN hPen, OldPen;
	TCHAR str[256];
	static TCHAR *Mes = TEXT("�¿�Ŭ��:�� ����,Space:����, ����:�ӵ�,E:����,A:�ڵ�");
	int i,t;
	RECT SCREEN = crt;
	
	MoveBall();
	MoveBullet();
	if ((t=HitCheck())!= -1) {									//��Ʈ���� ����
		SplitBall(t);											//���ɰ���
	}
	hdc = GetDC(hWndMain);
	if (hBit == NULL) {
		hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
	}
	hMemDC = CreateCompatibleDC(hdc);
	OldBit = (HBITMAP)SelectObject(hMemDC, hBit);
	FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));
	SelectObject(hMemDC, GetStockObject(NULL_PEN));
	for (i = 0; i < MAXBULLET; i++) {							//�Ѿ� �׸���
		if (arBullet[i].Exist) {
			TransBlt(hMemDC, arBullet[i].x, arBullet[i].y, hBullet, RGB(255, 255, 255));
		}
	}
	for (i = 0; i < MAXBALL; i++) {									//�� �׸���
		if (arBall[i].Exist) {
			hBrush = CreateSolidBrush(arBall[i].col);
			hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
			Ellipse(hMemDC, int(arBall[i].x - arBall[i].r), int(arBall[i].y - arBall[i].r), int(arBall[i].x + arBall[i].r), int(arBall[i].y + arBall[i].r));
			DeleteObject(SelectObject(hMemDC, hOldBrush));
		}
	}

	TransBlt(hMemDC, PlayerPos, crt.bottom - 20,hPlayer, RGB(255, 255, 255));				//�÷��̾� ǥ��	

	SetTextAlign(hMemDC, TA_CENTER);													//������
	wsprintf(str, TEXT("�������� = %d ���� = %d ���� =%d"), nStage, Life, Score);			
	TextOut(hMemDC, crt.right / 2, crt.top + 30, str, lstrlen(str));

	if(GetKeyState(VK_LEFT)&0x8000){				//�¿� Ű�Է�
		PlayerPos-=10;
	}
	if(GetKeyState(VK_RIGHT)&0x8000){
		PlayerPos+=10;
	}

	SelectObject(hMemDC, OldBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWndMain, hdc);
	GameEnd();

	SCREEN.bottom -= 101;
	InvalidateRect(hWndMain, &SCREEN, FALSE);
	SCREEN.bottom += 101;
	SCREEN.top = SCREEN.bottom - 99;
	InvalidateRect(hWndMain, &SCREEN, FALSE);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static BOOL CHECK = FALSE;
	int i;

	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;
		GetClientRect(hWnd, &crt); 
		PlayerPos = crt.right / 2;
		hPlayer = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		hBullet = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		srand(GetTickCount());
		return 0;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_SPACE:
			NewBullet(TRUE);
			break;
		case VK_RETURN:
			StartStage(TRUE);			
			for (i = 0; i < nStage; i++) {
				if (i % 2 == 0) NewBall(TRUE);
				else {
					NewBall(FALSE);
				}
			}
			CHECK = TRUE;
			TIMERCON = TRUE;
			break;
		}
		return 0;
	case WM_TIMER:
		if (TIMERCON == TRUE) {
			OnTimer();
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawBitmap(hdc, 0, 0, hBit);
		if (!CHECK) {
			TextOut(hdc, PlayerPos, crt.bottom - 20, TEXT("A"), 1);
			MoveToEx(hdc, crt.left, crt.bottom - 100, NULL);
			LineTo(hdc, crt.right, crt.bottom - 100);
			SetTextAlign(hdc, TA_CENTER);
			wsprintf(str, TEXT("�������� = %d ���� = %d ���� =%d"), nStage, Life, Score);
			TextOut(hdc, crt.right / 2, crt.top + 30, str, lstrlen(str));
		}
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		DeleteObject(hBit);
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void MoveBall()
{
	int i;
	tag_Ball B;

	for (i = 0; i < MAXBALL; i++) {
		if (arBall[i].Exist) {
			B = arBall[i];

			//X���� �̵� -dx�� �������׸�ŭ �����ϵ� 0�� ��踦 ���� ����
			if (B.dx >= 0) {
				B.dx = max(0, B.dx - AR);
			}
			else {
				B.dx = min(0, B.dx + AR);
			}
			B.x += B.dx;

			//������ ���� ���� - �������� ����Ͽ� �����ŭ �������� �ݻ�
			if (B.x + B.r >= crt.right) {
				B.x = crt.right - (B.x + B.r - crt.right) - B.r;
				B.dx *= -1;
			}
			//���� ���� ����
			if (B.x - B.r <= 0) {
				B.x = 0 - (B.x - B.r) + B.r;
				B.dx *= -1;
			}

			//Y���� �̵�
			//�ϰ��Ҷ� -dy�� ���ӵ���ŭ ��� ����.
			if (B.dy >= 0) {
				B.dy += A;
				//����� �� - ���ӵ��� A�� ź���� ���� �ݹ߷��� �Ǹ� �߷��� ������ �޴´�.
			}
			else {
				B.dy += A + G;
			}
			B.y += B.dy;

			/*//�ٴڿ� ���� ��� -dy�� ��ȣ�� �ٲپ� ���� Ƣ�� �������� �Ѵ�.
			if (B.y + B.r >= crt.bottom) {
				B.y = crt.bottom - (B.y + B.r - crt.bottom) - B.r;
				B.dy *= -1;
				//���� �������� �������� ���� �����Ѵ�.
				if (fabs(B.dy) < 1.0) {
					B.Exist = FALSE;
				}
			}

			//���� �������� �������� ���� �����Ѵ�.
			if (fabs(B.dx) < 0.1) {
				B.Exist = FALSE;
			}*/

			if (B.y + B.r >= crt.bottom - 100) {
				B.Exist = FALSE;
				Life--;
			}
			arBall[i] = B;

		}
	}
}

void NewBall(BOOL bLeft)
{
	int i;
	for (i = 0; i < nStage; i++) {
		if (arBall[i].Exist == FALSE) {
			break;
		}
	}

	if (i != nStage) {
		arBall[i].dx = 2.0 + (rand() % 20) / 10.0;
		if (bLeft) {
			arBall[i].x = 30;
		}
		else {
			arBall[i].x = crt.right;
			arBall[i].dx *= -1;
		}
		arBall[i].y = 20;
		arBall[i].dy = (rand() % 10) / 10.0;
		arBall[i].r = 40;
		arBall[i].col = RGB(rand() % 256, rand() % 256, rand() % 256);
		arBall[i].Exist = TRUE;
	}
}

void StartStage(BOOL Start)
{
	if (Start) {
		if (END == TRUE) {
			SetTimer(hWndMain, 1, 20, NULL);
			SendMessage(hWndMain, WM_TIMER, 1, NULL);
			Life = 5;
			Score = 0;
		}
		else {
			SetTimer(hWndMain, 1, 20, NULL);
			SendMessage(hWndMain, WM_TIMER, 1, NULL);
			Life = 5;
			Score = 0;
			PlayerPos = crt.right / 2;
		}
	}
}
void NewBullet(BOOL Shoot)
{
	int i;
	for (i = 0; i < MAXBULLET; i++) {
		if (arBullet[i].Exist == FALSE) {
			break;
		}
	}

	if (i != MAXBULLET) {
		if (Shoot) {
			arBullet[i].x = PlayerPos;
			arBullet[i].y = crt.bottom - 40;
			arBullet[i].Exist = TRUE;
		}
	}
}
void MoveBullet()
{
	int i;

	for (i = 0; i < MAXBULLET; i++) {
		if(arBullet[i].Exist){
			arBullet[i].y -= 10;
		}

		if (arBullet[i].y == crt.top) {
			arBullet[i].Exist = FALSE;
		}

	}
	//������ �������� crt.top�� ���������� BulletPos��y���� -1�� �̵� 
}

int HitCheck() 
{
	int i, j;

	for (i = 0; i < MAXBALL; i++) {
		for (j = 0; j < MAXBULLET; j++) {
			if(arBall[i].Exist==TRUE&&arBullet[j].Exist==TRUE&&(arBall[i].x+arBall[i].r>arBullet[j].x)&&(arBall[i].x-arBall[i].r<arBullet[j].x)
				&& (arBall[i].y + arBall[i].r > arBullet[j].y) && (arBall[i].y - arBall[i].r < arBullet[j].y)) {
				Score++;
				arBullet[j].Exist = FALSE;
				return i;
			}
		}
	}
	return -1;
}

void SplitBall(int t)
{
	int i,j;
	for (i = 0; i < MAXBALL; i++) {
		if (arBall[i].Exist == FALSE) {
			break;
		}
	}
	for (j = i + 1; j < MAXBALL; j++) {
		if (arBall[j].Exist == FALSE) {
			break;
		}
	}
	arBall[i].x = arBall[t].x;
	arBall[j].x = arBall[t].x;
	arBall[i].y = arBall[t].y;
	arBall[j].y = arBall[t].y;
	arBall[i].dx = 3;
	arBall[j].dx = -3;
	arBall[i].dy = -5;
	arBall[j].dy = -5;
	if (arBall[t].r - 10 == 0) {
		arBall[t].Exist = FALSE;
		return;
	}
	arBall[i].r = arBall[t].r - 10;
	arBall[j].r = arBall[t].r - 10;
	arBall[i].col = arBall[t].col;
	arBall[j].col = arBall[t].col;
	arBall[i].Exist = TRUE;
	arBall[j].Exist = TRUE;
	arBall[t].Exist = FALSE;
}

void GameEnd()
{
	int i;

	for (i = 0; i < MAXBALL; i++) {
		if (arBall[i].Exist) {
			END = FALSE;
			break;
		}
		else {
			END = TRUE;
		}
	}
	if (END == TRUE) {
		KillTimer(hWndMain, 1);
		MessageBox(hWndMain, TEXT("���� Ŭ����"), TEXT("Congratulation"), MB_OK);
		nStage++;
		TIMERCON = FALSE;
	}
}
