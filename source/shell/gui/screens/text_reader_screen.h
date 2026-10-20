#ifndef _3D_SHELL_TEXT_READER_SCREEN_H
#define _3D_SHELL_TEXT_READER_SCREEN_H

#include "gui.h"
#include "notepad.h"
#include "notepad_session.h"

#include <status_bar.h>
#include <touch_bar.h>
#include "screen.h"
#include "screen_manager.h"
#include "textures.h"

namespace GUI {

class TextReaderScreen : public UI::Screen {
public:
    const char *Name() const override { return "editor"; }
    const C2D_Image *Icon(bool) const override { return &icon_notepad; }
    TextReaderScreen(MenuItem *item);


    void OnFrame(u64 delta_time) override;
    void DrawTop() override;
    void DrawBottom() override;
    void OnKeyDown(u32 k_down) override;
    void OnKeyHeld(u32 k_held) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    MenuItem *item_;
    u32 k_held_ = 0;
    bool bar_touching_ = false;
    NotepadSession session_;
    StatusBar status_bar_;
    TouchBar touch_bar_;
};

}

#endif
