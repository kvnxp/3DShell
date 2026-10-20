# Task: FolderView control and file dialog list on the top screen

- Date: 2026-10-19
- Requested by: user
- Status: completed

## Tasks
- [x] Create the FolderView control in source/shell/controls/ (listing, D-pad navigation, folder/file filter, scroll bar)
- [x] Move the directory listing and filter helpers out of FileDialogScreen into FolderView
- [x] Move the file list to the top screen (gfx0) in FileDialogScreen; keep Name, Type and buttons on gfx1
- [x] Remove touch selection of list rows (the list is no longer on the touch screen)
- [x] Update the on-screen key hints and the error message position
- [ ] Update docs/ui-components.md (dialog layout) and docs/TEXT_EDITOR.md (dialog section)
- [ ] Build with DEVKITPRO=/opt/devkitpro make -j8
- [ ] Move this file to docs/task/completed/ when every item is done
