# 3DShell — Project Rules

## Git

- **Never commit or push unless the user explicitly asks for it in that turn.** The user decides when changes get committed ("yo te digo cuándo hacé commits"). No automatic commits, no proactive `git commit`/`git push`.
- Conventional Commits, no AI attribution.

## Language

- All app UI strings, assets and code comments in **English** — nothing in the app may be in Spanish.

## Task Management

- Every user instruction that involves work must be turned into a task list and tracked with checkboxes, marking each item as it gets completed (both via the session todo tool and in the file below).
- Each new instruction/request generates a Markdown file in `docs/task/` with its task list. File naming: `YYYY-MM-DD-short-description.md` (e.g. `docs/task/2026-10-18-add-dark-mode.md`).
- File template:

  ```markdown
  # Task: <short description of the instruction>

  - Date: YYYY-MM-DD
  - Requested by: user
  - Status: in progress | completed

  ## Tasks
  - [ ] Step 1
  - [ ] Step 2
  ```

- Check off items (`- [x]`) in the file **immediately** as each task is completed — not in a batch at the end.
- Update the `Status` line to `completed` when every checkbox is done.
- When all checkboxes in a task file are done, move that file to `docs/task/completed/` (create the directory if it does not exist). The file keeps its original name.
- Do not delete or rewrite previous task files; they are the project's work log.

## Search

- `tgrep` is available for fast trigram-indexed regex search over the codebase. Build/rebuild the index with `tgrep index` when needed; fall back to grep/glob if unavailable.

## Architecture (in progress)

- `source/ui/` = reusable UI framework (WinForms-style; must not depend on app code).
- `source/shell/` = 3DShell application code (desktop shell + panel apps + screens).
- UI is transitioning from immediate-mode drawing to retained-mode live controls; system functions go through the app registry (`include/apps.h`).
