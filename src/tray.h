#pragma once
#include <napi.h>
#include <memory>
#include <thread>
#include <functional>
#include <unordered_map>
#include "n-utils.hpp"
#include "utils.hpp"

class NodeTray : public Napi::ObjectWrap<NodeTray>
{
    static Napi::FunctionReference constructor_;

    std::wstring iconPath_;
    HICON icon_ = nullptr;

    std::shared_ptr<std::thread> worker_;
    HWND window_ = nullptr;
    POINT cursor_;
    bool mouseEntered_ = false;

    std::unordered_map<std::string, std::shared_ptr<NodeEventCallback>> callbacks_;

    template<class ...Arg>
    void async_call_node(const std::string& eventName, const Arg& ... args)
    {
        auto it = callbacks_.find(eventName);
        if (it != callbacks_.end())
        {
            auto cb = it->second;

            Napi::HandleScope scope(cb->env);

            cb->callback.MakeCallback(cb->receiver.Value(), { Napi::Value::From(cb->env, args)... });
        }
    }


  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    NodeTray(const Napi::CallbackInfo &info);
    ~NodeTray();

    Napi::Value setEventCallback(const Napi::CallbackInfo &info);
    Napi::Value destroy(const Napi::CallbackInfo &info);
    Napi::Value setIcon(const Napi::CallbackInfo &info);
    Napi::Value setToolTip(const Napi::CallbackInfo &info);
    Napi::Value getBounds(const Napi::CallbackInfo &info);

    LRESULT _windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void _handleClick(bool left_mouse_click, bool double_button_click);
    void _handleMouseMove();
    void _checkMouseLeave();

    void onMouseEnter();
    void onMouseLeave();

    void onDoubleClicked();
    void onClicked();
    void onRightClicked();

    void start();
    void stop();

    void _createMessageWindow();
    void _destroyWindow();

    void loadIcon();
    void destroyIcon();

    bool addTrayIcon();
    bool destroyTrayIcon();
    bool updateIcon(const std::string& icon);
    bool updateToolTip(const std::string& tip);

    RECT getRect();

    HICON getIcon();
};
