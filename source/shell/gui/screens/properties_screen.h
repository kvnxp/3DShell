#ifndef _3D_SHELL_PROPERTIES_SCREEN_H
#define _3D_SHELL_PROPERTIES_SCREEN_H

#include "gui.h"

#include "screen.h"
#include "textures.h"
#include "screen_manager.h"

namespace GUI {

class PropertiesScreen : public UI::Screen {
public:
    const char *Name() const override { return "properties-dialog"; }
    const C2D_Image *Icon(bool dark_theme) const override { return dark_theme ? &dialog_dark : &dialog; }
    PropertiesScreen(UI::Screen *owner, MenuItem *item);


    void OnFrame(u64 delta_time) override;
    void DrawTop() override;
    void DrawBottom() override;
    void OnKeyDown(u32 k_down) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    void Close();

    UI::Screen *owner_;
    MenuItem *item_;
    UI::Rect ok_rect_;
    bool touching_ = false;
    bool bar_touching_ = false;
};

}

#endif
