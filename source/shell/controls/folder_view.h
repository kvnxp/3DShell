#ifndef _3D_SHELL_FOLDER_VIEW_H
#define _3D_SHELL_FOLDER_VIEW_H

#include <3ds.h>

#include <functional>
#include <string>
#include <vector>

#include <control.h>
#include "controls/scroll_bar.h"

namespace GUI {

/* Folder list drawn inside its bounds and driven by the D-pad. Shows folders
 * and, optionally, files that match the extension filter. The host screen
 * decides what B does, because at the root it may need to cancel. */
class FolderView : public UI::Control {
public:
    struct Palette {
        u32 background;
        u32 text;
        u32 selector;
        u32 scroll_track;
        u32 scroll_thumb;
    };

    FolderView(const UI::Rect &bounds, size_t rows, const Palette &palette);

    /* Returns false and keeps the current folder when the path does not exist. */
    bool Navigate(const std::string &path);
    /* Returns false when already at the root. */
    bool GoParent();

    void SetShowFiles(bool show);
    /* Extensions without the leading dot. An empty list shows every file. */
    void SetExtensions(const std::vector<std::string> &extensions);

    /* Always ends with '/'. Empty until the first successful Navigate(). */
    const std::string &Cwd() const { return cwd_; }

    /* Fires after a successful Navigate() or a folder entered with A. */
    std::function<void()> Changed;
    /* Fires with the file name when A is pressed on a file. */
    std::function<void(const std::string &name)> FileChosen;
    /* Fires with a message when a folder cannot be opened. */
    std::function<void(const std::string &message)> Failed;

    void Draw() override;
    void OnKeyDown(u32 k_down) override;

private:
    struct Item {
        std::string name;
        bool is_dir;
    };

    void Refresh();
    void EnsureSelectedVisible();
    void Activate();

    std::string cwd_;
    bool show_files_ = true;
    std::vector<std::string> extensions_;
    std::vector<Item> items_;
    size_t selected_ = 0;
    size_t top_ = 0;
    size_t rows_;
    float row_h_;
    Palette palette_;
    UI::ScrollBar scroll_bar_;
};

}

#endif
