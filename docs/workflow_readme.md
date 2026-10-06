# CI Workflow Reference

This document describes the GitHub Actions workflows in `.github/workflows/`. Each workflow has a single coherent purpose; this file explains the orchestration that's not obvious from the YAML alone.

## tests.yml — Regression tests

Triggered on push and pull_request. Runs the pytest suite (`tests/`) on Linux, macOS (x86_64 + arm64), and Windows (MSYS2/MINGW64), for both CLI and GUI build variants. The full pytest suite is gated:

- **PR / push events**: fast tests only. Runs in ~5–10 minutes per platform.
- **Scheduled cron, release tags (`v*`), and manual `workflow_dispatch`**: full suite including slow tests (large-image RS02 / RS03i tests).

Slow tests are marked with `@pytest.mark.slow` in `tests/test_rs02.py` and `tests/test_rs03i.py`. The `--run-slow` pytest option (defined in `tests/conftest.py`) opts them in; CI passes it conditionally.

The first test run on any host creates ~3GB of master images in `/var/tmp/regtest/` and reuses them on subsequent runs. CI caches this directory between runs to save startup time.

## release.yml — Multi-platform release builds

Triggered on push to master/dev and on `v*` tags. Produces:

- `linux64-cli` — static CLI binary
- `linux64-deb` — Debian package
- `linux64-appimage` — AppImage built inside `ubuntu:18.04` Docker container
- `win (cli)` / `win (gui)` — Windows binaries via MSYS2/MINGW64
- `mac (cli, x86_64 / arm64)` / `mac (gui, x86_64 / arm64)` — macOS binaries

A `prepare-tag` job runs first to avoid tag race conditions between parallel platform builds.

### AppImage build note

The AppImage job intentionally builds inside `ubuntu:18.04` (Bionic) to keep the resulting AppImage's glibc dependency at 2.27 — the lowest common denominator across modern Linux distros, which maximizes downstream compatibility.

Bionic reached end-of-standard-support on 2023-04-30. The main, updates, and backports apt pockets were moved off `archive.ubuntu.com` to `old-releases.ubuntu.com`. The "install prerequisites in docker" step rewrites `/etc/apt/sources.list` to point at `old-releases.ubuntu.com` before `apt update` so package installation succeeds. The `bionic-security` pocket on `security.ubuntu.com` is also rewritten for consistency.

## codeql.yml — Static analysis

Triggered on push, pull_request, and weekly cron. Runs CodeQL static analysis. No special configuration.

## stale.yml — Issue housekeeping

Triggered on daily cron. Auto-closes issues labeled `needs-more-info` or `answered` after a quiet period.
