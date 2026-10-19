# Task: Touch bar as tab bar (apps add tabs)

- Date: 2026-10-19
- Requested by: user
- Status: completed

## Tasks
- [x] Add `Remove()` to `UI::ContainerControl`
- [x] Add `TabButton` + tab API to `TouchBar` (AddTab/RemoveTab/SetActiveTab/ActiveContent/Tabs)
- [x] Migrate `DesktopScreen` panel system to tabs (drawer special, apps become tabs)
- [x] Route OnFrame/OnTouch/OnKeyDown to active tab content
- [x] Validate on hardware: drawer opens, app opens as tab, tab stays active, X closes tab
