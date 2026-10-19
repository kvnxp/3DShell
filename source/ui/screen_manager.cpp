#include "screen_manager.h"
#include "task_manager_panel.h"
#include "c2d_helper.h"

#include <3ds.h>

namespace UI {

ScreenManager &ScreenManager::Instance() {
    static ScreenManager instance;
    return instance;
}

void ScreenManager::Run(std::unique_ptr<Screen> root) {
    stack_.clear();
    graveyard_.clear();
    stack_.push_back(std::move(root));
    running_ = true;

    u64 last_time = osGetTime();
    last_touch_ = {};

    while (running_ && aptMainLoop() && !stack_.empty()) {
        u64 current_time = osGetTime();
        delta_time_ = current_time - last_time;
        last_time = current_time;

        if (frame_hook_)
            frame_hook_();

        Screen *screen = Current();
        if (!screen)
            break;

        screen->OnFrame(delta_time_);
        screen = Current();
        if (!screen)
            break;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top_screen, screen->TopBackground());
        C2D_TargetClear(bottom_screen, screen->BottomBackground());
        C2D_SceneBegin(top_screen);
        screen->DrawTop();
        C2D_SceneBegin(bottom_screen);
        screen->DrawBottom();
        
        // Draw task manager overlay if active
        DrawTaskManagerOverlay();
        
        C2D::Render();

        hidScanInput();

        u32 k_down = hidKeysDown();
        u32 k_held = hidKeysHeld();

        // Handle SELECT key to toggle task manager (global hotkey)
        if (k_down & KEY_SELECT) {
            ToggleTaskManager();
        }

        if (Current() == screen)
            screen->OnKeyHeld(k_held);

        // Route key events to task manager if active, otherwise to current screen
        if (task_manager_) {
            HandleTaskManagerKey(k_down);
        } else if (Current() == screen) {
            screen->OnKeyDown(k_down);
        }

        touchPosition touch = last_touch_;
        bool touching = (k_held & KEY_TOUCH) != 0;
        if (touching) {
            hidTouchRead(&touch);
            last_touch_ = touch;
        }

        // Route touch events to task manager if active, otherwise to current screen
        if (task_manager_) {
            HandleTaskManagerTouch(touch, touching);
        } else if (Current() == screen) {
            screen->OnTouch(touch, touching);
        }

        graveyard_.clear();
    }

    graveyard_.clear();
    stack_.clear();
    running_ = false;
}

void ScreenManager::Push(std::unique_ptr<Screen> screen) {
    if (screen)
        stack_.push_back(std::move(screen));
}

void ScreenManager::Pop() {
    if (stack_.size() > 1) {
        graveyard_.push_back(std::move(stack_.back()));
        stack_.pop_back();
    }
}

Screen *ScreenManager::Current() const {
    return stack_.empty() ? nullptr : stack_.back().get();
}

const Screen *ScreenManager::Focused() const {
    return Current();
}

bool ScreenManager::IsCurrent(const Screen *screen) const {
    return Current() == screen;
}

Screen *ScreenManager::GetScreenAt(size_t index) const {
    if (index < stack_.size()) {
        return stack_[index].get();
    }
    return nullptr;
}

void ScreenManager::CloseScreenAt(size_t index) {
    if (index > 0 && index < stack_.size()) {
        graveyard_.push_back(std::move(stack_[index]));
        stack_.erase(stack_.begin() + index);
    }
}

void ScreenManager::SwitchToScreenAt(size_t index) {
    if (index > 0 && index < stack_.size()) {
        // Move the screen to the top of the stack
        auto screen = std::move(stack_[index]);
        stack_.erase(stack_.begin() + index);
        stack_.push_back(std::move(screen));
    }
}

void ScreenManager::ToggleTaskManager() {
    if (task_manager_) {
        task_manager_->Close();
        task_manager_.reset();
        WindowClosed();
    } else {
        task_manager_ = std::make_unique<GUI::TaskManagerPanel>();
        task_manager_->Closed = [this]() {
            task_manager_.reset();
            WindowClosed();
        };
        WindowOpened();
    }
}

void ScreenManager::DrawTaskManagerOverlay() {
    if (task_manager_) {
        task_manager_->Draw();
    }
}

void ScreenManager::HandleTaskManagerKey(u32 k_down) {
    if (task_manager_) {
        task_manager_->OnKeyDown(k_down);
    }
}

void ScreenManager::HandleTaskManagerTouch(const touchPosition &touch, bool down) {
    if (task_manager_) {
        task_manager_->OnTouch(touch, down);
    }
}

}

UI::ScreenManager::ScreenManager() = default;
UI::ScreenManager::~ScreenManager() = default;
