#include "tray.h"
#include <assert.h>
#include "utils/node_async_call.h"
#include <iostream>

#define CHECK_RESULT(result)                                    \
    do                                                          \
    {                                                           \
        if (!result)                                            \
        {                                                       \
            std::cerr << "err:" << GetLastError() << std::endl; \
        }                                                       \
    } while (0)

static const UINT g_WndMsgTaskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");

#define TRAY_WINDOW_MESSAGE (WM_USER + 100)
#define WAKEUP_MESSAGE (WM_USER + 101)


static UINT nextTrayIconId()
{
    static UINT next_id = 2;
    return next_id++;
}

HICON getIconHandle(const std::wstring &iconPath)
{
    return (HICON)(HICON)::LoadImage(NULL, iconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
}

//////////////////////////////////////////////////////////////////////////

class DummyWindow
{
  public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        NodeTray *pThis = nullptr;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
            pThis = (NodeTray *)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = (NodeTray *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (pThis)
        {
            return pThis->_windowProc(hwnd, uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
};

class WindowClassRegister
{
  public:
    WindowClassRegister() { ; }
    ~WindowClassRegister()
    {
        for (std::vector<std::wstring>::iterator i = m_windowClass.begin(); i != m_windowClass.end(); ++i)
        {
            ::UnregisterClassW(i->c_str(), nullptr);
        }
    }

    static WindowClassRegister &instance()
    {
        static WindowClassRegister rr;
        return rr;
    }

    bool registerWindowClass(const std::wstring &cls)
    {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc = DummyWindow::WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = cls.c_str();
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

        ATOM r = RegisterClassEx(&wc);
        assert(r && "registerClass");
        return !!r;
    }

    bool registerClass(const std::wstring &cls)
    {
        if (isRegistered(cls))
        {
            return true;
        }

        if (registerWindowClass(cls))
        {
            m_windowClass.push_back(cls);
            return true;
        }
        else
            return false;
    }

    bool isRegistered(const std::wstring &cls)
    {
        return std::find(m_windowClass.begin(), m_windowClass.end(), cls) != m_windowClass.end();
    }

  private:
    std::vector<std::wstring> m_windowClass;
};

//////////////////////////////////////////////////////////////////////////

Napi::FunctionReference NodeTray::constructor_;

Napi::Object NodeTray::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "NodeTray", {
                                                           NAPI_METHOD_INSTANCE(NodeTray, destroy),
                                                           NAPI_METHOD_INSTANCE(NodeTray, setIcon),
                                                           NAPI_METHOD_INSTANCE(NodeTray, setToolTip),
                                                           NAPI_METHOD_INSTANCE(NodeTray, getBounds),
                                                       });

    constructor_ = Napi::Persistent(func);
    constructor_.SuppressDestruct();

    exports.Set("NodeTray", func);
    return exports;
}

NodeTray::NodeTray(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<NodeTray>(info)
    , id_(nextTrayIconId())
{
    wrapper_ = Napi::Weak(info.This().ToObject());
    iconPath_ = utils::fromUtf8(info[0].ToString());
    start();
}

NodeTray::~NodeTray()
{
    stop();
}

Napi::Value NodeTray::destroy(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    stop();
    return env.Undefined();
}

Napi::Value NodeTray::setIcon(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    std::string icon = info[0].ToString();

    updateIcon(icon);

    return env.Undefined();
}

Napi::Value NodeTray::setToolTip(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    std::string tip = info[0].ToString();

    updateToolTip(tip);
    return env.Undefined();
}

Napi::Value NodeTray::getBounds(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    RECT rect = getRect();

    Napi::Object obj = Napi::Object::New(env);

    obj.Set("x", Napi::Value::From(env, rect.left));
    obj.Set("y", Napi::Value::From(env, rect.top));
    obj.Set("width", Napi::Value::From(env, rect.right - rect.left));
    obj.Set("height", Napi::Value::From(env, rect.bottom - rect.top));

    return obj;
}

LRESULT NodeTray::_windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (TRAY_WINDOW_MESSAGE == uMsg)
    {
        switch (LOWORD(lParam))
        {
        case NIN_BALLOONUSERCLICK:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_CONTEXTMENU:
            _handleClick(
                (lParam == WM_LBUTTONDOWN || lParam == NIN_BALLOONUSERCLICK || lParam == WM_LBUTTONDBLCLK),
                (lParam == WM_LBUTTONDBLCLK || lParam == WM_RBUTTONDBLCLK));
            return TRUE;
            break;
        case WM_MOUSEMOVE:
            _handleMouseMove();
            break;
        default:
            break;
        }

        return TRUE;
    }

    if (g_WndMsgTaskbarCreated == uMsg)
    {
        addTrayIcon();
        return TRUE;
    }

    if (uMsg == WM_TIMER)
    {
        _checkMouseLeave();
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void NodeTray::_handleClick(bool leftClick, bool doubleClick)
{
    if (leftClick)
    {
        if (doubleClick)
        {
            node_async_call::async_call([this]() {
                this->onDoubleClicked();
            });
        }
        else
        {
            node_async_call::async_call([this]() {
                this->onClicked();
            });
        }

        return;
    }
    else if (!doubleClick)
    {
        node_async_call::async_call([this]() {
            this->onRightClicked();
        });
    }
}

void NodeTray::_handleMouseMove()
{
    GetCursorPos(&cursor_);
    if (!mouseEntered_)
    {
        RECT rect = getRect();
        if (cursor_.x < rect.left || cursor_.x > rect.right || cursor_.y < rect.top || cursor_.y > rect.bottom)
        {
            return;
        }
        else
        {
            mouseEntered_ = true;
            node_async_call::async_call([this]() {
                this->onMouseEnter();
            });
        }
    }
}

void NodeTray::_checkMouseLeave()
{
    if (mouseEntered_)
    {
        POINT cursor;
        GetCursorPos(&cursor);
        RECT rect = getRect();
        if (cursor.x < rect.left || cursor.x > rect.right || cursor.y < rect.top || cursor.y > rect.bottom)
        {
            mouseEntered_ = false;
            node_async_call::async_call([this]() {
                this->onMouseLeave();
            });
        }
    }
}

void NodeTray::onMouseEnter()
{
    this->emit("mouse-enter");
}

void NodeTray::onMouseLeave()
{
    this->emit("mouse-leave");
}

void NodeTray::onDoubleClicked()
{
    this->emit("double-click");
}

void NodeTray::onClicked()
{
    this->emit("click");
}

void NodeTray::onRightClicked()
{
    this->emit("right-click");
}

void NodeTray::start()
{
    HANDLE ready = CreateEventW(nullptr, true, false, nullptr);

    worker_ = std::make_shared<std::thread>([this, ready]() {
        _createMessageWindow();

        DWORD timer = ::SetTimer(window_, 0, 200, nullptr);

        SetEvent(ready);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        KillTimer(window_, timer);

        _destroyWindow();
    });

    WaitForSingleObject(ready, INFINITE);
    CloseHandle(ready);

    loadIcon();
    addTrayIcon();
}

void NodeTray::stop()
{
    if (worker_)
    {
        destroyTrayIcon();
        destroyIcon();

        PostThreadMessage(GetThreadId(worker_->native_handle()), WM_QUIT, NULL, NULL);
        worker_->join();
        worker_.reset();
    }
}

void NodeTray::_createMessageWindow()
{
    static const WCHAR windowclass[] = L"node_trayicon_msg_window_2a0b1c8d";

    WindowClassRegister::instance().registerClass(windowclass);

    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

    WNDCLASSEXW wc;
    wc.cbSize = sizeof(wc);
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = NULL;
    wc.hInstance = hInstance;
    wc.lpfnWndProc = DummyWindow::WindowProc;
    wc.lpszClassName = windowclass;
    wc.lpszMenuName = NULL;
    wc.style = 0;

    window_ = CreateWindowExW(
        0,
        windowclass,
        L"trayicon_msg_window",
        WS_POPUP,
        0, 0, 0, 0,
        NULL,
        NULL,
        hInstance,
        this);
}

void NodeTray::_destroyWindow()
{
    if (window_)
    {
        DestroyWindow(window_);
        window_ = nullptr;
    }
}

void NodeTray::loadIcon()
{
    destroyIcon();
    icon_ = getIconHandle(iconPath_);
}

void NodeTray::destroyIcon()
{
    if (icon_)
    {
        ::DestroyIcon(icon_);
        icon_ = nullptr;
    }
}

bool NodeTray::addTrayIcon()
{
    NOTIFYICONDATAW icon_data = {0};
    getInitializedNCD(icon_data);

    icon_data.uFlags = NIF_ICON | NIF_MESSAGE;
    icon_data.hIcon = icon_;
    icon_data.uCallbackMessage = TRAY_WINDOW_MESSAGE;
    BOOL result = Shell_NotifyIcon(NIM_ADD, &icon_data);
    return !!result;
}

bool NodeTray::destroyTrayIcon()
{
    NOTIFYICONDATAW icon_data = {0};
    getInitializedNCD(icon_data);
    BOOL result = Shell_NotifyIcon(NIM_DELETE, &icon_data);
    CHECK_RESULT(result);
    return !!result;
}

bool NodeTray::updateIcon(const std::string &icon)
{
    iconPath_ = utils::fromUtf8(icon);
    loadIcon();

    NOTIFYICONDATAW icon_data = {0};
    getInitializedNCD(icon_data);
    icon_data.uFlags |= NIF_ICON;
    icon_data.hIcon = icon_;

    BOOL result = Shell_NotifyIconW(NIM_MODIFY, &icon_data);
    CHECK_RESULT(result);
    return !!result;
}

bool NodeTray::updateToolTip(const std::string &tip)
{
    NOTIFYICONDATAW icon_data = {0};
    getInitializedNCD(icon_data);
    icon_data.uFlags |= NIF_TIP;
    wcsncpy_s(icon_data.szTip, utils::fromUtf8(tip).c_str(), _TRUNCATE);
    BOOL result = Shell_NotifyIcon(NIM_MODIFY, &icon_data);
    CHECK_RESULT(result);
    return !!result;
}

RECT NodeTray::getRect()
{
    NOTIFYICONIDENTIFIER icon_id;
    memset(&icon_id, 0, sizeof(NOTIFYICONIDENTIFIER));
    icon_id.uID = id_;
    icon_id.hWnd = window_;
    icon_id.cbSize = sizeof(NOTIFYICONIDENTIFIER);
    RECT rect = {0};
    Shell_NotifyIconGetRect(&icon_id, &rect);

    return rect;
}

HICON NodeTray::getIcon()
{
    return icon_;
}

void NodeTray::getInitializedNCD( NOTIFYICONDATAW &ncd)
{
    ncd.cbSize = sizeof(NOTIFYICONDATAW);
    ncd.hWnd = window_;
    ncd.uID = id_;
}
