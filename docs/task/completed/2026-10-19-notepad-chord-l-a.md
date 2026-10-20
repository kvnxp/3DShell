# Task: Replace Notepad SELECT with L+A chord

- Date: 2026-10-19
- Requested by: user
- Status: completed

## Tasks
- [x] Add a reusable chord helper in `source/ui/` (hold a modifier, press a button)
- [x] Notepad edit mode: L+A opens the system keyboard to edit the current line (replaces the dead SELECT branch)
- [x] Remove the unreachable SELECT branch in Notepad
- [x] Update `docs/TEXT_EDITOR.md` (controls and the SELECT conflict note)
- [x] Update `README.md` if it lists Notepad controls (checked: it does not list them, no change needed)
- [x] Build with `DEVKITPRO=/opt/devkitpro make -j8`
- [x] Image viewer: L+Y zoom in and L+B zoom out (held); L no longer navigates, D-Left/Right and R do
- [x] Image viewer: B exit ignored while L is held, so L+B does not close the viewer
- [x] Image viewer: L+X resets zoom (replaces the unreachable SELECT reset), and L+X does not open Properties
- [x] Image viewer: D-Left/Right repeat while held (400 ms delay, then every 120 ms)
- [x] Build with `DEVKITPRO=/opt/devkitpro make -j8`
- [x] Update `docs/ui-components.md` for the viewer controls
