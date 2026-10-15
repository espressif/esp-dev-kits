# Ordering Information Tables

`ordering_info.py` keeps the **Ordering Information** table of every dev kit in sync with the Espressif product API (<https://products.espressif.com/api/user/devkits>).

The table lists every variant of a board with its part number, on-board module, flash, PSRAM, and antenna. The script only writes `ordering-info.inc`. Adding `.. include:: ordering-info.inc` to a user guide is done by hand.

## Usage

All commands read current data from the product API. The repository does not store an API snapshot.

### Single board or boards in an MR

Use these commands so only the boards you are editing are rewritten. Unrelated tables stay out of the MR. The pre-commit hook refreshes a board when a staged file is `user_guide*.rst`. It skips an EOL or other-target board, and it only warns when the API has no match. `apply --board` stops in both of those cases.

```bash
# Preview one board against the live API
python tools/ordering_info/ordering_info.py report --board esp32-c6-devkitc-1

# Write that board's English and Chinese inc files
python tools/ordering_info/ordering_info.py apply --board esp32-c6-devkitc-1

# Write inc files for every board whose docs changed relative to master
python tools/ordering_info/ordering_info.py apply --since origin/master

# Fail if those boards' inc files are missing or stale (CI on merge requests)
python tools/ordering_info/ordering_info.py check --since origin/master
```

`apply --since` includes uncommitted files. `check --since` compares the ref with `HEAD` only. If `ordering_info.py`, `config.json`, `eol-boards.rst`, or `other-index.inc` changed, `apply --since` stops and asks for a full `apply`; `check --since` checks every board.

### Scheduled full refresh

These commands process every board. They belong to the weekly schedule, not to a normal board MR.

```bash
# Preview every board against the live API
python tools/ordering_info/ordering_info.py report

# Write every stale inc file
python tools/ordering_info/ordering_info.py apply

# Fail if any board's inc file is missing or stale
python tools/ordering_info/ordering_info.py check
```

## CI jobs

`check_ordering_info` calls the product API but does not write files. It compares committed `ordering-info.inc` files with current API data:

- On a **merge request**, it checks boards whose files changed in that MR. A stale table outside that diff does not fail the job. If the diff changes the generator, `config.json`, or an exclusion list, every board is checked.
- On **master**, it checks the boards in that push. If the previous commit SHA is missing, it checks every board.

`refresh_ordering_info` is the global refresh. It runs only from a GitLab **Pipeline schedule** that sets `ORDERING_INFO_REFRESH=1`. Set the cadence (for example, every Friday) in **CI/CD → Schedules**. The job refreshes every board and, if anything changed, pushes `ci/ordering-info-refresh` and opens an MR.

Schedule settings:

- Target branch: `master`
- Variable: `ORDERING_INFO_REFRESH` = `1`
- Optional variable: `SYNC_MR_ASSIGNEE` = GitLab username to assign the generated MR

The job has to push a branch, so it needs a credential that may write to the repository. Set `ORDERING_INFO_TOKEN` to a project access token with the `write_repository` scope and at least the Developer role. Without that variable, the job falls back to the `GITLAB_KEY` SSH key from the global `before_script`, which only works if that key has write access and is available on the branch the schedule runs on.

## Adding a new user guide / updating one board

Install the repository hooks once:

```bash
pre-commit install
```

When a commit contains `docs/en/<folder>/user_guide*.rst` or its Chinese counterpart, the hook queries `?name=<board>` and refreshes that board's English and Chinese inc files. If the named query is empty, it reads the full catalog and matches the board name with spaces and punctuation removed. If a generated file changes, the commit stops. Stage the reported inc files and commit again. A merge request runs this hook and the separate `check_ordering_info` job, so bypassing the hook locally does not bypass CI. The check job reports stale files and does not write them.

For a new guide, add `.. include:: ordering-info.inc` where the table should appear. The hook creates the inc file on the first commit attempt. The English guide folder is required for board discovery; a Chinese guide is generated when a matching Chinese file exists.

If the folder name cannot be matched automatically, add an entry to `name_overrides` in `config.json` and commit again. To exclude a board entirely, add it to `exclude_folders`.

## How it works

For every user guide that has API data, the script writes an `ordering-info.inc` file in the same folder:

```
docs/en/esp32-c6-devkitc-1/ordering-info.inc
docs/zh_CN/esp32-c6-devkitc-1/ordering-info.inc
```

The inc file is a plain RST `Ordering Information` section with a `list-table`. To show it in a user guide, add:

```rst
.. include:: ordering-info.inc
```

Never edit the inc file manually. Each file starts with an RST comment naming this script, and the hook or `apply` overwrites it from the live API. To change wording or column layout, edit `config.json` or the `TEXT` dict in the script.

Each generated heading has the RST target `ordering-info-<folder>`, such as `ordering-info-esp32-s2-devkitc-1`. English and Chinese use the same name. The target stays in the inc file, on the heading. A target in the user guide directly before the include does not give a `:ref:` a title.

The table heading copies the underline of that guide's Ordering Information heading. If the guide has no such heading, it copies the Retail Orders / 零售订单 underline. If neither heading exists, it uses the third level in that guide's heading order. When a guide already includes the inc file, `check` expands the include and reports a title-level mismatch before Sphinx does.

Part numbers are listed with mass-production variants first. Non-production statuses are marked in the part number column: `(Sample)`, `(NRND)`, `(EOL)` in English and `（样品）`, `（NRND）`, `（停产）` in Chinese. The API field `Replaced` maps to NRND.

## What the script leaves alone

- **User guides.** `apply` never edits `.rst` files. Existing include directives stay as they are.
- **Hand-written table.** A guide that already has its own `Ordering Information` section is left unchanged. The inc file is still generated alongside it. `report` prints a column-by-column diff, and `check` prints those differences as warnings. A stale inc file does not fail `check` while the hand-written section remains.
- **EOL boards.** Boards reachable from `docs/en/eol/eol-boards.rst` are skipped.
- **"Other"-target boards.** Boards listed in `docs/en/index/other-index.inc` (programming/flashing tools such as ESP-Prog) are skipped.
- **Boards missing from the API.** `report` lists these.
- **Superseded revisions.** Only the current guide of a folder is used to generate the inc file: `user_guide.rst` if it exists, otherwise the highest-numbered `user_guide_vX.Y.rst`.
