# Task: Show app icons in the Task Manager

- Date: 2026-10-19
- Requested by: user
- Status: completed

## Tasks
- [x] Add a virtual `Icon(bool dark_theme)` accessor to `UI::Screen` that returns `nullptr` by default
- [x] Override `Icon` in the File Explorer, Notepad and Text editor screens (same icons as the App Drawer)
- [x] Override `Icon` in the dialog screens (file dialog, delete, options, properties) with the generic dialog icon
- [x] Store the icon in `TaskManagerPanel::AppInfo` and draw it scaled to the row, shifting the row text to make room
- [x] Build with `DEVKITPRO=/opt/devkitpro make -j8`
- [x] Update `docs/ui-components.md` if it documents the Task Manager (it does not; README line updated instead)
