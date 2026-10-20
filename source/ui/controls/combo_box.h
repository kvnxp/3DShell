#ifndef _3D_SHELL_UI_COMBO_BOX_H
#define _3D_SHELL_UI_COMBO_BOX_H

#include <functional>
#include <string>
#include <vector>

#include "../control.h"
#include "../c2d_helper.h"

namespace UI {

// Dropdown selector. The list opens above the box when drop_up is set. Draw it
// last in its owner so the open list covers the controls underneath.
class ComboBox : public Control {
public:
    struct Palette {
        u32 background;
        u32 text;
        u32 border;
        u32 highlight;
    };

    ComboBox(const Rect &bounds, const std::vector<std::string> &items,
             const Palette &palette, bool drop_up = false)
        : items_(items), palette_(palette), drop_up_(drop_up) {
        bounds_ = bounds;
    }

    void Draw() override {
        DrawBox(bounds_.x, bounds_.y, bounds_.w, bounds_.h, palette_.background);
        if (!items_.empty()) {
            C2D::Text(bounds_.x + PADDING, bounds_.y + (bounds_.h - ITEM_TEXT_H) / 2.0f,
                      TEXT_SIZE, palette_.text, items_[selected_]);
        }
        C2D::Text(bounds_.x + bounds_.w - ARROW_W, bounds_.y + (bounds_.h - ITEM_TEXT_H) / 2.0f,
                  TEXT_SIZE, palette_.text, open_ ? "^" : "v");

        if (!open_)
            return;

        Rect list = ListRect();
        DrawBox(list.x, list.y, list.w, list.h, palette_.background);
        for (size_t i = 0; i < items_.size(); i++) {
            Rect row = RowRect(i);
            if (static_cast<int>(i) == selected_)
                C2D::Rect(row.x, row.y, row.w, row.h, palette_.highlight);
            C2D::Text(row.x + PADDING, row.y + (row.h - ITEM_TEXT_H) / 2.0f,
                      TEXT_SIZE, palette_.text, items_[i]);
        }
    }

    // Touch acts on the down edge. Outside touches while open only close the list.
    void OnTouch(const touchPosition &touch, bool down) override {
        if (!down) {
            touching_ = false;
            return;
        }
        if (touching_)
            return;
        touching_ = true;

        if (!open_) {
            if (bounds_.Contains(touch))
                open_ = true;
            return;
        }

        open_ = false;
        if (items_.empty())
            return;
        Rect list = ListRect();
        if (!list.Contains(touch))
            return;
        for (size_t i = 0; i < items_.size(); i++) {
            if (RowRect(i).Contains(touch)) {
                SetSelected(static_cast<int>(i));
                return;
            }
        }
    }

    bool IsOpen() const { return open_; }
    void Close() { open_ = false; }

    int Selected() const { return selected_; }

    void SetSelected(int index) {
        if (index >= 0 && index < static_cast<int>(items_.size()) && index != selected_) {
            selected_ = index;
            if (Changed)
                Changed(selected_);
        }
    }

    // Moves the selection by delta, wrapping around. Fires Changed.
    void Step(int delta) {
        if (items_.empty())
            return;
        int count = static_cast<int>(items_.size());
        int next = ((selected_ + delta) % count + count) % count;
        selected_ = next;
        if (Changed)
            Changed(selected_);
    }

    std::function<void(int)> Changed;

private:
    static constexpr float PADDING = 6.0f;
    static constexpr float ROW_HEIGHT = 18.0f;
    static constexpr float ARROW_W = 14.0f;
    static constexpr float ITEM_TEXT_H = 14.0f;
    static constexpr float TEXT_SIZE = 0.5f;
    static constexpr float BORDER = 1.0f;

    void DrawBox(float x, float y, float w, float h, u32 fill) {
        C2D::Rect(x, y, w, h, palette_.border);
        C2D::Rect(x + BORDER, y + BORDER, w - BORDER * 2.0f, h - BORDER * 2.0f, fill);
    }

    Rect ListRect() const {
        Rect r;
        r.x = bounds_.x;
        r.w = bounds_.w;
        r.h = ROW_HEIGHT * static_cast<float>(items_.size());
        r.y = drop_up_ ? bounds_.y - r.h : bounds_.y + bounds_.h;
        return r;
    }

    Rect RowRect(size_t index) const {
        Rect list = ListRect();
        Rect r;
        r.x = list.x;
        r.w = list.w;
        r.h = ROW_HEIGHT;
        r.y = list.y + ROW_HEIGHT * static_cast<float>(index);
        return r;
    }

    std::vector<std::string> items_;
    Palette palette_;
    bool drop_up_;
    int selected_ = 0;
    bool open_ = false;
    bool touching_ = false;
};

}

#endif
