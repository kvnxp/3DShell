#include "folder_view.h"

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <locale>

#include "c2d_helper.h"
#include "colours.h"
#include "fs.h"
#include "smdh.h"
#include "textures.h"

namespace GUI {

namespace {

constexpr float SCROLL_W = 4.0f;
constexpr float TEXT_SIZE = 0.45f;
constexpr float CHAR_W = 7.0f;
constexpr float TEXT_PAD = 4.0f;
constexpr float ICON_X = 4.0f;
constexpr float ICON_SIZE = 16.0f;
constexpr float TEXT_X = 26.0f;

std::size_t U16Len(const u16 *src) {
    std::size_t len = 0;
    while (src[len])
        len++;
    return len;
}

/* Entry names are fixed-size UTF-16 arrays. Surrogates are dropped, as in the
 * File Explorer, so the conversion never throws. */
std::string Utf16ToUtf8(const u16 *src) {
    std::u16string clean;
    std::size_t len = U16Len(src);
    for (std::size_t i = 0; i < len; i++) {
        char16_t c = src[i];
        if ((c >= 0xD800) && (c <= 0xDFFF))
            continue;
        clean.push_back(c);
    }

    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(clean);
}

std::u16string Utf8ToUtf16(const std::string &src) {
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(src);
}

bool HasExtension(const std::string &name, const std::string &ext) {
    if (name.size() <= ext.size() + 1)
        return false;
    if (name[name.size() - ext.size() - 1] != '.')
        return false;
    for (std::size_t i = 0; i < ext.size(); i++) {
        char a = static_cast<char>(std::tolower(static_cast<unsigned char>(name[name.size() - ext.size() + i])));
        char b = static_cast<char>(std::tolower(static_cast<unsigned char>(ext[i])));
        if (a != b)
            return false;
    }
    return true;
}

bool MatchesExtensions(const std::string &name, const std::vector<std::string> &extensions) {
    if (extensions.empty())
        return true;
    for (const std::string &ext : extensions) {
        if (HasExtension(name, ext))
            return true;
    }
    return false;
}

std::string HeadText(const std::string &text, std::size_t max) {
    if (text.size() <= max)
        return text;
    return text.substr(0, max - 3) + "...";
}

}

FolderView::FolderView(const UI::Rect &bounds, size_t rows, const Palette &palette)
    : rows_(rows > 0 ? rows : 1),
      palette_(palette),
      scroll_bar_(UI::Rect{bounds.x + bounds.w - SCROLL_W, bounds.y, SCROLL_W, bounds.h},
                  palette.scroll_track, palette.scroll_thumb) {
    bounds_ = bounds;
    row_h_ = bounds.h / static_cast<float>(rows_);
}

bool FolderView::Navigate(const std::string &path) {
    std::string dir = path;
    if (dir.empty() || dir.back() != '/')
        dir += '/';

    if (!FS::DirExists(archive, dir)) {
        if (Failed)
            Failed("Folder not found.");
        return false;
    }

    cwd_ = dir;
    Refresh();
    if (Changed)
        Changed();
    return true;
}

bool FolderView::GoParent() {
    if (cwd_ == "/")
        return false;

    std::string trimmed = cwd_.substr(0, cwd_.size() - 1);
    return Navigate(trimmed.substr(0, trimmed.find_last_of('/') + 1));
}

void FolderView::SetShowFiles(bool show) {
    show_files_ = show;
    Refresh();
}

void FolderView::SetExtensions(const std::vector<std::string> &extensions) {
    extensions_ = extensions;
    Refresh();
}

void FolderView::Refresh() {
    items_.clear();

    if (!cwd_.empty()) {
        std::vector<FS_DirectoryEntry> entries;
        FS::GetDirList(cwd_, entries, -1);
        for (const FS_DirectoryEntry &entry : entries) {
            std::string name = Utf16ToUtf8(entry.name);
            if (name.empty())
                continue;

            if (entry.attributes & FS_ATTRIBUTE_DIRECTORY) {
                items_.push_back(Item{name, true, FileTypeNone, {}});
                continue;
            }

            if (!show_files_ || !MatchesExtensions(name, extensions_))
                continue;

            FileType type = FS::GetFileType(name);
            std::u16string smdh_path;
            if (type == FileType3dsx)
                smdh_path = Utf8ToUtf16(cwd_ + name);
            items_.push_back(Item{name, false, type, smdh_path});
        }
    }

    selected_ = 0;
    top_ = 0;
    scroll_bar_.SetRange(static_cast<int>(items_.size()), static_cast<int>(rows_));
    scroll_bar_.SetValue(0);
}

void FolderView::EnsureSelectedVisible() {
    if (selected_ < top_)
        top_ = selected_;
    else if (selected_ >= top_ + rows_)
        top_ = selected_ + 1 - rows_;
    scroll_bar_.SetValue(static_cast<int>(top_));
}

void FolderView::Activate() {
    if (selected_ >= items_.size())
        return;

    /* Copied because Navigate() replaces items_. */
    const Item item = items_[selected_];
    if (item.is_dir) {
        Navigate(cwd_ + item.name + "/");
        return;
    }

    if (FileChosen)
        FileChosen(item.name);
}

void FolderView::OnKeyDown(u32 k_down) {
    if (k_down & KEY_UP) {
        if (selected_ > 0)
            selected_--;
        EnsureSelectedVisible();
    } else if (k_down & KEY_DOWN) {
        if (selected_ + 1 < items_.size())
            selected_++;
        EnsureSelectedVisible();
    } else if (k_down & KEY_A) {
        Activate();
    }
}

void FolderView::PumpIcons() {
    std::vector<std::u16string> wanted;
    size_t last = std::min(items_.size(), top_ + rows_);
    for (size_t i = top_; i < last; i++) {
        if (!items_[i].smdh_path.empty())
            wanted.push_back(items_[i].smdh_path);
    }

    if (!wanted.empty())
        SMDH::Pump(wanted);
}

void FolderView::DrawIcon(const C2D_Image &image, float x, float y) const {
    float scale = ICON_SIZE / static_cast<float>(image.subtex->width);
    C2D::ImageScale(image, x, y, scale, scale);
}

void FolderView::Draw() {
    if (!visible_)
        return;

    C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, palette_.background);

    float list_w = bounds_.w - SCROLL_W;
    std::size_t max_chars = static_cast<std::size_t>((list_w - TEXT_X - TEXT_PAD) / CHAR_W);

    if (items_.empty())
        C2D::Text(bounds_.x + TEXT_PAD, bounds_.y + 2.0f, TEXT_SIZE, palette_.text, "(empty)");

    for (size_t row = 0; row < rows_; row++) {
        size_t index = top_ + row;
        if (index >= items_.size())
            break;

        float y = bounds_.y + row_h_ * static_cast<float>(row);
        if (index == selected_)
            C2D::Rect(bounds_.x, y, list_w, row_h_, palette_.selector);

        const Item &item = items_[index];

        const C2D_Image *icon = &file_icons[item.type];
        if (item.is_dir) {
            icon = palette_.dark_icons? &icon_dir_dark : &icon_dir;
        } else if (item.type == FileType3dsx) {
            SMDHMeta *meta = nullptr;
            if (SMDH::Peek(item.smdh_path, &meta) && meta->has_icon)
                icon = &meta->icon;
        }
        DrawIcon(*icon, bounds_.x + ICON_X, y + (row_h_ - ICON_SIZE) / 2.0f);

        std::string label = HeadText(item.name + (item.is_dir? "/" : ""), max_chars);
        C2D::Text(bounds_.x + TEXT_X, y + 2.0f, TEXT_SIZE, palette_.text, label);
    }

    scroll_bar_.Draw();
}

}
