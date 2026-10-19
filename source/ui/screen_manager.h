#ifndef _3D_SHELL_UI_SCREEN_MANAGER_H
#define _3D_SHELL_UI_SCREEN_MANAGER_H

#include <memory>
#include <vector>

#include <3ds.h>

#include "screen.h"

namespace GUI {
    class TaskManagerPanel;
}

namespace UI {

class ScreenManager {
public:
    static ScreenManager &Instance();

    void Run(std::unique_ptr<Screen> root);
    void Push(std::unique_ptr<Screen> screen);
    void Pop();
    void Exit() { running_ = false; }

    bool IsCurrent(const Screen *screen) const;

    /* Window-manager queries: fullscreen apps live on the screen stack
     * (root excluded); panel apps register/unregister with the calls
     * below. Chrome (close buttons) derives from these. */
    int WindowCount() const { return static_cast<int>(stack_.size()) - 1 + panel_windows_; }
    bool HasWindow() const { return WindowCount() > 0; }
    void WindowOpened() { ++panel_windows_; }
    void WindowClosed() { if (panel_windows_ > 0) --panel_windows_; }

    /* Debug-channel queries. */
    const Screen *Focused() const;
    const touchPosition &LastTouch() const { return last_touch_; }

    /* Shell per-frame hook (kernel poll, etc). */
    void SetFrameHook(void (*hook)(void)) { frame_hook_ = hook; }

    u64 DeltaTime() const { return delta_time_; }

    /* Task Manager overlay (global, SELECT to toggle). */
    void ToggleTaskManager();
    void DrawTaskManagerOverlay();
    void HandleTaskManagerKey(u32 k_down);
    void HandleTaskManagerTouch(const touchPosition &touch, bool down);
    
    /* Task Manager stack inspection methods. */
    size_t StackSize() const { return stack_.size(); }
    Screen *GetScreenAt(size_t index) const;
    void CloseScreenAt(size_t index);
    void SwitchToScreenAt(size_t index);

private:
    ScreenManager();
    ~ScreenManager();

    Screen *Current() const;

    std::vector<std::unique_ptr<Screen>> stack_;
    std::vector<std::unique_ptr<Screen>> graveyard_;
    int panel_windows_ = 0;
    bool running_ = false;
    u64 delta_time_ = 0;
    touchPosition last_touch_ = {};
    void (*frame_hook_)() = nullptr;

    std::unique_ptr<GUI::TaskManagerPanel> task_manager_;
};

}

#endif
