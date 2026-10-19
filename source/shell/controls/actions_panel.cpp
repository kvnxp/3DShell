#include "actions_panel.h"

#include <codecvt>
#include <locale>

#include "file_explorer_screen.h"
#include "delete_screen.h"
#include "properties_screen.h"
#include "osk.h"

#include <c2d_helper.h>
#include <colours.h>
#include <config.h>
#include <fs.h>
#include <screen_manager.h>
#include <textures.h>
#include <touch.h>
#include <utils.h>

#include "gui.h"

namespace GUI {

/* Clipboard state: survives panel closes (copy/move two-step flow). */
static bool copy = false;
static bool move = false;

ActionsPanel::ActionsPanel(FileExplorerScreen *owner, MenuItem *item) 
    : owner_(owner), item_(item) {}

void ActionsPanel::OnFrame(u64 delta_time) {
    (void)delta_time;
}

void ActionsPanel::Draw() {
    C2D::Rect(0, 20, 400, 35, cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar
    C2D::Rect(0, 55, 320, 185, cfg.dark_theme? BLACK_BG : WHITE);

    C2D::Text(10, 30, 0.44f, WHITE, "Actions");

    // Grid layout: 3 columns x 3 rows
    const int grid_x = 20;
    const int grid_y = 60;
    const int cell_w = 90;
    const int cell_h = 40;
    const int gap_x = 10;
    const int gap_y = 5;
    
    int row = 0;
    int col = 0;
    
    // Draw grid cells
    for (int i = 0; i < COUNT; i++) {
        int x = grid_x + col * (cell_w + gap_x);
        int y = grid_y + row * (cell_h + gap_y);
        
        // Selection highlight
        if (i == selection_) {
            C2D::Rect(x - 2, y - 2, cell_w + 4, cell_h + 4, 
                      cfg.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
        }
        
        // Cell background
        C2D::Rect(x, y, cell_w, cell_h, 
                  cfg.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
        
        // Cell text
        const char* label = nullptr;
        switch (i) {
            case NEW_FILE: label = "New File"; break;
            case NEW_FOLDER: label = "New Folder"; break;
            case PROPERTIES: label = "Properties"; break;
            case RENAME: label = "Rename"; break;
            case COPY: label = copy ? "Paste" : "Copy"; break;
            case PASTE: label = move ? "Paste" : "Move"; break;
            case MOVE: label = "Move"; break;
            case DELETE_ACTION: label = "Delete"; break;
            case REFRESH: label = "Refresh"; break;
        }
        
        if (label) {
            float text_width = 0.0f;
            C2D::GetTextSize(0.40f, &text_width, nullptr, label);
            C2D::Text(x + (cell_w - text_width) / 2, y + 15, 0.40f, 
                      cfg.dark_theme? WHITE : BLACK, label);
        }
        
        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }
}

void ActionsPanel::OnKeyHeld(u32 k_held) {
    (void)k_held;
    Touch::Update();
}

void ActionsPanel::OnKeyDown(u32 k_down) {
    // Navigation
    int row = selection_ / 3;
    int col = selection_ % 3;
    
    if (k_down & KEY_DUP) {
        if (row > 0) selection_ -= 3;
    } else if (k_down & KEY_DDOWN) {
        if (row < 2 && selection_ + 3 < COUNT) selection_ += 3;
    } else if (k_down & KEY_DLEFT) {
        if (col > 0) selection_--;
    } else if (k_down & KEY_DRIGHT) {
        if (col < 2 && selection_ + 1 < COUNT) selection_++;
    }

    if (k_down & KEY_A)
        PerformAction(static_cast<Action>(selection_));
    else if (k_down & KEY_B)
        Close();
}

void ActionsPanel::OnTouch(const touchPosition &touch, bool down) {
    if (!down)
        return;

    // Grid layout: 3 columns x 3 rows
    const int grid_x = 20;
    const int grid_y = 60;
    const int cell_w = 90;
    const int cell_h = 40;
    const int gap_x = 10;
    const int gap_y = 5;
    
    for (int i = 0; i < COUNT; i++) {
        int row = i / 3;
        int col = i % 3;
        int x = grid_x + col * (cell_w + gap_x);
        int y = grid_y + row * (cell_h + gap_y);
        
        if (Touch::Rect(x, y, x + cell_w, y + cell_h)) {
            selection_ = i;
            PerformAction(static_cast<Action>(i));
            return;
        }
    }
}

void ActionsPanel::PerformAction(Action action) {
    switch (action) {
        case NEW_FILE: {
            std::string path = cfg.cwd;
            std::string name = OSK::GetText("New File", "Enter file name");
            if (!name.empty()) {
                path.append(name);
                std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());

                if (R_SUCCEEDED(FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0, 0))) {
                    if (ListingChanged)
                        ListingChanged();
                }
            }
            break;
        }

        case NEW_FOLDER: {
            std::string path = cfg.cwd;
            std::string name = OSK::GetText("New Folder", "Enter folder name");
            if (!name.empty()) {
                path.append(name);
                std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(path.data());

                if (R_SUCCEEDED(FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, path_u16.c_str()), 0))) {
                    if (ListingChanged)
                        ListingChanged();
                }
            }
            break;
        }

        case PROPERTIES: {
            UI::ScreenManager::Instance().Push(std::make_unique<PropertiesScreen>(owner_, item_));
            break;
        }

        case RENAME: {
            const std::u16string entry_name_utf16 = reinterpret_cast<const char16_t *>(item_->entries[item_->selected].name);
            const std::string filename = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(entry_name_utf16.data());

            std::string new_name = OSK::GetText(filename, "Enter new name");
            if (!new_name.empty()) {
                if (R_SUCCEEDED(FS::Rename(&item_->entries[item_->selected], new_name.c_str()))) {
                    if (ListingChanged)
                        ListingChanged();
                }
            }
            break;
        }

        case COPY: {
            if (!copy) {
                // First press: copy
                if ((item_->checked_count >= 1) && (item_->checked_cwd.compare(cfg.cwd) != 0))
                    ResetCheckbox(item_);
                if (item_->checked_count <= 1)
                    FS::Copy(&item_->entries[item_->selected], cfg.cwd);
                copy = true;
            } else {
                // Second press: paste
                if ((item_->checked_count > 1) && (item_->checked_cwd.compare(cfg.cwd) != 0)) {
                    // Handle multiple paste
                    Result ret = 0;
                    std::vector<FS_DirectoryEntry> entries;
                    if (R_FAILED(ret = FS::GetDirList(item_->checked_cwd.data(), entries)))
                        return;
                    for (u32 i = 0; i < item_->checked_copy.size(); i++) {
                        if (item_->checked_copy.at(i)) {
                            FS::Copy(&entries[i], item_->checked_cwd);
                            if (R_FAILED(FS::Paste())) {
                                FS::GetDirList(cfg.cwd, item_->entries);
                                ResetCheckbox(item_);
                                break;
                            }
                        }
                    }
                    FS::GetDirList(cfg.cwd, item_->entries);
                    ResetCheckbox(item_);
                } else {
                    if (R_SUCCEEDED(FS::Paste())) {
                        FS::GetDirList(cfg.cwd, item_->entries);
                        ResetCheckbox(item_);
                    }
                }
                RecalcStorageSize(item_);
                copy = false;
                if (ListingChanged)
                    ListingChanged();
            }
            break;
        }

        case PASTE: {
            if (!move) {
                // First press: move
                if ((item_->checked_count >= 1) && (item_->checked_cwd.compare(cfg.cwd) != 0))
                    ResetCheckbox(item_);
                if (item_->checked_count <= 1)
                    FS::Copy(&item_->entries[item_->selected], cfg.cwd);
                move = true;
            } else {
                // Second press: paste move
                if ((item_->checked_count > 1) && (item_->checked_cwd.compare(cfg.cwd) != 0)) {
                    // Handle multiple move
                    Result ret = 0;
                    std::vector<FS_DirectoryEntry> entries;
                    if (R_FAILED(ret = FS::GetDirList(item_->checked_cwd.data(), entries)))
                        return;
                    for (u32 i = 0; i < item_->checked_copy.size(); i++) {
                        if (item_->checked_copy.at(i)) {
                            FS::Copy(&entries[i], item_->checked_cwd);
                            if (R_FAILED(FS::Move())) {
                                FS::GetDirList(cfg.cwd, item_->entries);
                                ResetCheckbox(item_);
                                break;
                            }
                        }
                    }
                    FS::GetDirList(cfg.cwd, item_->entries);
                    ResetCheckbox(item_);
                } else if (R_SUCCEEDED(FS::Move())) {
                    FS::GetDirList(cfg.cwd, item_->entries);
                    ResetCheckbox(item_);
                }
                move = false;
                if (ListingChanged)
                    ListingChanged();
            }
            break;
        }

        case MOVE: {
            // Same as PASTE for now
            if (!move) {
                if ((item_->checked_count >= 1) && (item_->checked_cwd.compare(cfg.cwd) != 0))
                    ResetCheckbox(item_);
                if (item_->checked_count <= 1)
                    FS::Copy(&item_->entries[item_->selected], cfg.cwd);
                move = true;
            } else {
                if ((item_->checked_count > 1) && (item_->checked_cwd.compare(cfg.cwd) != 0)) {
                    Result ret = 0;
                    std::vector<FS_DirectoryEntry> entries;
                    if (R_FAILED(ret = FS::GetDirList(item_->checked_cwd.data(), entries)))
                        return;
                    for (u32 i = 0; i < item_->checked_copy.size(); i++) {
                        if (item_->checked_copy.at(i)) {
                            FS::Copy(&entries[i], item_->checked_cwd);
                            if (R_FAILED(FS::Move())) {
                                FS::GetDirList(cfg.cwd, item_->entries);
                                ResetCheckbox(item_);
                                break;
                            }
                        }
                    }
                    FS::GetDirList(cfg.cwd, item_->entries);
                    ResetCheckbox(item_);
                } else if (R_SUCCEEDED(FS::Move())) {
                    FS::GetDirList(cfg.cwd, item_->entries);
                    ResetCheckbox(item_);
                }
                move = false;
                if (ListingChanged)
                    ListingChanged();
            }
            break;
        }

        case DELETE_ACTION: {
            UI::ScreenManager::Instance().Push(std::make_unique<DeleteScreen>(owner_, item_));
            break;
        }

        case REFRESH: {
            if (ListingChanged)
                ListingChanged();
            break;
        }

        default:
            break;
    }
}

void ActionsPanel::ShowMoreOptions() {
    more_options_shown_ = true;
}

void ActionsPanel::HideMoreOptions() {
    more_options_shown_ = false;
}

}
