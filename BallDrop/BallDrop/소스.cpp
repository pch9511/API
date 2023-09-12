#include <windows.h>
#include <math.h>
#define MAXBALL 1000   //���� �ִ� ����
#define MAXBULLET 10
const double A = 0.15; //�߷� ���ӵ�
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
const int cy = 360;
tag_Ball arBall[MAXBALL];
HBITMAP hBit;
HWND hWndMain;
RECT crt;

void MoveBall();
void NewBall(BOOL bLeft);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("First");

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static TCHAR *Mes = TEXT("�¿�Ŭ��:�� ����,Space:����, ����:�ӵ�,E:����,A:�ڵ�");
	static BOOL bPause;
	static BOOL bAuto;
	static int Freq;
	int i;
	HBRUSH hBrush, hOldBrush;
	HDC hMemDC;
	HBITMAP OldBit;

	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;
		GetClientRect(hWnd, &crt);
		srand(GetTickCount());
		Freq = 20;
		SetTimer(hWnd,1,Freq, NULL);
		SendMessage(hWnd, WM_TIMER, 1, NULL);
		bPause = FALSE;
		return 0;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_UP:
			Freq = max(10, Freq - 5);
			SetTimer(hWnd, 1, Freq, NULL);
			break;
		case VK_DOWN:
			Freq = min(200, Freq + 5);
			SetTimer(hWnd, 1, Freq, NULL);
			break;
		case ' ':
			if (bPause) {
				SetTimer(hWnd, 1, Freq, NULL);
			}
			else {
				KillTimer(hWnd, 1);
			}
			bPause = !bPause;
			break;
		case 'E':
			for (i = 0; i < MAXBALL; i++) {
				arBall[i].Exist = FALSE;
			}
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case 'A':
			bAuto = !bAuto;
			break;
		}
		return 0;
	case WM_TIMER:
		if (bAuto) {
			if (rand() % 8 == 0) NewBall(TRUE);
			if (rand() % 9 == 0)NewBall(FALSE);
		}
		MoveBall();
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &crt);
		if (hBit == NULL) {
			hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
		}
		hMemDC = CreateCompatibleDC(hdc);
		OldBit = (HBITMAP)SelectObject(hMemDC, hBit);
		FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));
		SelectObject(hMemDC, GetStockObject(NULL_PEN));

		for (i = 0; i < MAXBALL; i++) {
			if (arBall[i].Exist) {
				hBrush = CreateSolidBrush(arBall[i].col);
				hOldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
				Ellipse(hMemDC, int(arBall[i].x - arBall[i].r), int(arBall[i].y - arBall[i].r), int(arBall[i].x + arBall[i].r), int(arBall[i].y + arBall[i].r));
				DeleteObject(SelectObject(hMemDC, hOldBrush));
			}
		}
		SetTextAlign(hMemDC, TA_CENTER);
		TextOut(hMemDC, crt.right/2, 10, Mes, lstrlen(Mes));
		SelectObject(hMemDC, OldBit);
		DeleteDC(hMemDC);
		ReleaseDC(hWnd, hdc);
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_LBUTTONDOWN:
		if (!bPause) NewBall(TRUE);
		return 0;
	case WM_RBUTTONDOWN:
		if (!bPause) NewBall(FALSE);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawBitmap(hdc, 0, 0, hBit);
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
	//RECT crt;

	GetClientRect(hWndMain, &crt);
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

			//�ٴڿ� ���� ��� -dy�� ��ȣ�� �ٲپ� ���� Ƣ�� �������� �Ѵ�.
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
			}
			arBall[i] = B;
		}
	}
}

void NewBall(BOOL bLeft)
{
	int i;
	for (i = 0; i < MAXBALL; i++) {
		if (arBall[i].Exist == FALSE) {
			break;
		}
	}

	if (i != MAXBALL) {
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