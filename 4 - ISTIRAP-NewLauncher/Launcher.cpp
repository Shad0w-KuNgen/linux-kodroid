// ISTIRAP Launcher
//
// Clean, from-scratch launcher for the ISTIRAP Knight Online server.
// Talks to our own LoginServer using the existing patch/version protocol
// (opcodes 0x1/0x2/0x3 - see LauncherEngine.cpp / LogInServer/LoginSession.cpp),
// then starts KnightOnLine.exe once the client is confirmed up to date.
//
// There is no IP allow-list or hardware-lock gate here: this launcher is
// meant to work with whatever server IP is configured in Server.ini.

#include "stdafx.h"
#include <objidl.h>
#include <gdiplus.h>
#include "LauncherEngine.h"
#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;

Launcher* Engine = nullptr;

static const wchar_t* kAssetDir = L"ISTIRAP\\Launcher\\";
static const wchar_t* kWindowClass = L"ISTIRAPLauncherWindow";
static const wchar_t* kWindowTitle = L"ISTIRAP Launcher";

static const int kWindowWidth = 992;
static const int kWindowHeight = 576;

struct ButtonSpec
{
    RECT rect;
    Bitmap* out = nullptr;
    Bitmap* over = nullptr;
    Bitmap* down = nullptr;
};

enum ButtonId
{
    BTN_START = 0,
    BTN_OPTIONS,
    BTN_CLOSE,
    BTN_COUNT
};

enum ButtonVisualState { VS_OUT, VS_OVER, VS_DOWN };

static ButtonSpec g_buttons[BTN_COUNT];
static ButtonVisualState g_buttonState[BTN_COUNT] = { VS_OUT, VS_OUT, VS_OUT };
static int g_pressedButton = -1;

static Bitmap* g_background = nullptr;
static Bitmap* g_progressEmpty = nullptr;
static Bitmap* g_progressValue = nullptr;
static RECT g_progressRect = { 24, 557, 24 + 979, 557 + 27 };

static HWND g_hwnd = nullptr;
static ULONG_PTR g_gdiplusToken = 0;

std::wstring AssetPath(const wchar_t* name)
{
    return std::wstring(kAssetDir) + name;
}

Bitmap* LoadAsset(const wchar_t* name)
{
    Bitmap* bmp = new Bitmap(AssetPath(name).c_str());
    if (bmp->GetLastStatus() != Ok)
    {
        delete bmp;
        return nullptr;
    }
    return bmp;
}

bool LoadAllAssets()
{
    g_background = LoadAsset(L"background.png");

    g_buttons[BTN_START].rect = { 608, 509, 608 + 296, 509 + 71 };
    g_buttons[BTN_START].out = LoadAsset(L"StartMouseOut.png");
    g_buttons[BTN_START].over = LoadAsset(L"StartMouseOver.png");
    g_buttons[BTN_START].down = LoadAsset(L"StartMouseClick.png");

    g_buttons[BTN_OPTIONS].rect = { 161, 511, 161 + 123, 511 + 30 };
    g_buttons[BTN_OPTIONS].out = LoadAsset(L"OptionsMouseOut.png");
    g_buttons[BTN_OPTIONS].over = LoadAsset(L"OptionsMouseOver.png");
    g_buttons[BTN_OPTIONS].down = LoadAsset(L"OptionsMouseClick.png");

    g_buttons[BTN_CLOSE].rect = { 765, 3, 765 + 30, 3 + 23 };
    g_buttons[BTN_CLOSE].out = LoadAsset(L"CloseMouseOut.png");
    g_buttons[BTN_CLOSE].over = LoadAsset(L"CloseMouseOver.png");
    g_buttons[BTN_CLOSE].down = LoadAsset(L"CloseMouseClick.png");

    g_progressEmpty = LoadAsset(L"ProgressEmpty.png");
    g_progressValue = LoadAsset(L"ProgressValue.png");

    return g_background != nullptr;
}

void FreeAllAssets()
{
    delete g_background;
    delete g_progressEmpty;
    delete g_progressValue;
    for (auto& b : g_buttons)
    {
        delete b.out;
        delete b.over;
        delete b.down;
    }
}

bool PtInRect2(const RECT& r, int x, int y)
{
    return x >= r.left && x < r.right && y >= r.top && y < r.bottom;
}

int HitTestButton(int x, int y)
{
    for (int i = 0; i < BTN_COUNT; i++)
        if (PtInRect2(g_buttons[i].rect, x, y))
            return i;
    return -1;
}

void PaintWindow(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    // Off-screen buffer to avoid flicker.
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    {
        Graphics g(memDC);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

        if (g_background)
            g.DrawImage(g_background, 0, 0, kWindowWidth, kWindowHeight);
        else
            g.Clear(Color(255, 20, 20, 20));

        for (int i = 0; i < BTN_COUNT; i++)
        {
            Bitmap* img = g_buttons[i].out;
            if (g_buttonState[i] == VS_DOWN) img = g_buttons[i].down;
            else if (g_buttonState[i] == VS_OVER) img = g_buttons[i].over;

            if (img)
            {
                RECT& r = g_buttons[i].rect;
                g.DrawImage(img, (INT)r.left, (INT)r.top, (INT)(r.right - r.left), (INT)(r.bottom - r.top));
            }
        }

        if (g_progressEmpty)
            g.DrawImage(g_progressEmpty, (INT)g_progressRect.left, (INT)g_progressRect.top,
                (INT)(g_progressRect.right - g_progressRect.left), (INT)(g_progressRect.bottom - g_progressRect.top));

        uint8 percent = Engine ? Engine->GetPercent() : 0;
        if (g_progressValue && percent > 0)
        {
            int fullWidth = g_progressRect.right - g_progressRect.left;
            int fullHeight = g_progressRect.bottom - g_progressRect.top;
            int w = (fullWidth * percent) / 100;
            if (w > 0)
            {
                Rect destRect(g_progressRect.left, g_progressRect.top, w, fullHeight);
                Rect srcRect(0, 0, (INT)(g_progressValue->GetWidth() * (percent / 100.0)), g_progressValue->GetHeight());
                g.DrawImage(g_progressValue, destRect, srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, UnitPixel);
            }
        }

        std::string state = Engine ? Engine->GetState() : "";
        if (!state.empty())
        {
            std::wstring wstate(state.begin(), state.end());
            FontFamily fontFamily(L"Segoe UI");
            Font font(&fontFamily, 13, FontStyleRegular, UnitPixel);
            SolidBrush brush(Color(255, 255, 255, 255));
            PointF origin((REAL)g_progressRect.left, (REAL)(g_progressRect.top - 20));
            g.DrawString(wstate.c_str(), -1, &font, origin, &brush);
        }
    }

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(hWnd, &ps);
}

void HandleButtonAction(int button)
{
    switch (button)
    {
    case BTN_START:
        if (Engine && Engine->IsReady())
        {
            if (Engine->LaunchGame())
                PostMessage(g_hwnd, WM_CLOSE, 0, 0);
        }
        break;
    case BTN_OPTIONS:
        ShellExecuteA(NULL, NULL, "Option.exe", NULL, NULL, SW_SHOWNORMAL);
        break;
    case BTN_CLOSE:
        PostMessage(g_hwnd, WM_CLOSE, 0, 0);
        break;
    }
}

DWORD WINAPI ConnectThreadProc(LPVOID)
{
    Engine->Start();
    InvalidateRect(g_hwnd, NULL, FALSE);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 200, NULL);
        break;

    case WM_TIMER:
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_PAINT:
        PaintWindow(hWnd);
        break;

    case WM_ERASEBKGND:
        return 1;

    case WM_MOUSEMOVE:
    {
        int x = LOWORD(lParam), y = HIWORD(lParam);
        int hit = HitTestButton(x, y);
        bool changed = false;
        for (int i = 0; i < BTN_COUNT; i++)
        {
            ButtonVisualState want = VS_OUT;
            if (i == g_pressedButton) want = VS_DOWN;
            else if (i == hit) want = VS_OVER;

            if (i == BTN_START && !(Engine && Engine->IsReady()) && g_pressedButton != BTN_START)
                want = VS_OUT;

            if (g_buttonState[i] != want)
            {
                g_buttonState[i] = want;
                changed = true;
            }
        }
        if (changed) InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam), y = HIWORD(lParam);
        int hit = HitTestButton(x, y);
        if (hit == BTN_START && !(Engine && Engine->IsReady()))
            hit = -1;
        g_pressedButton = hit;
        if (hit >= 0)
        {
            g_buttonState[hit] = VS_DOWN;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        SetCapture(hWnd);
    }
    break;

    case WM_LBUTTONUP:
    {
        ReleaseCapture();
        int x = LOWORD(lParam), y = HIWORD(lParam);
        int hit = HitTestButton(x, y);
        int pressed = g_pressedButton;
        g_pressedButton = -1;
        if (pressed >= 0)
            g_buttonState[pressed] = VS_OUT;
        InvalidateRect(hWnd, NULL, FALSE);

        if (pressed >= 0 && pressed == hit)
            HandleButtonAction(pressed);
    }
    break;

    case WM_SOCKETMSG:
    {
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_CONNECT:
            break;
        case FD_CLOSE:
            Engine->SetState("Disconnected.");
            Engine->SetPercent(0);
            break;
        case FD_READ:
        {
            Engine->mSocket->Receive();
            while (!Engine->mSocket->m_qRecvPkt.empty())
            {
                Packet* pkt = Engine->mSocket->m_qRecvPkt.front();
                Engine->HandlePacket(*pkt);
                delete pkt;
                Engine->mSocket->m_qRecvPkt.pop();
            }
            break;
        }
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    Engine = new Launcher();

    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = kWindowClass;
    wcex.hIcon = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(101), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    wcex.hIconSm = wcex.hIcon;

    if (!RegisterClassExW(&wcex))
        return 1;

    RECT wr = { 0, 0, kWindowWidth, kWindowHeight };
    AdjustWindowRect(&wr, WS_POPUP, FALSE);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - (wr.right - wr.left)) / 2;
    int y = (screenH - (wr.bottom - wr.top)) / 2;

    g_hwnd = CreateWindowExW(0, kWindowClass, kWindowTitle, WS_POPUP,
        x, y, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

    if (!g_hwnd)
        return 1;

    Engine->window = g_hwnd;

    if (!LoadAllAssets())
    {
        MessageBoxA(g_hwnd, "Could not load launcher assets from ISTIRAP\\Launcher\\. Make sure the launcher is run from the game's install folder.", "ISTIRAP Launcher", MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    // Only start connecting once Engine->window is set, since CAPISocket::Connect()
    // registers WSAAsyncSelect against that HWND - starting this any earlier (e.g.
    // from WM_CREATE, which fires during CreateWindowExW above) would race against
    // Engine->window still being NULL and WM_SOCKETMSG would never be delivered.
    CreateThread(NULL, 0, ConnectThreadProc, NULL, 0, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    FreeAllAssets();
    delete Engine;
    GdiplusShutdown(g_gdiplusToken);

    return (int)msg.wParam;
}
