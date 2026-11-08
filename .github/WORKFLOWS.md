# Workflow dependencies

Actions use full stable version tags in the workflows. Their exact commits and
repository identities are recorded in `.github/workflows/actions.lock`, generated
by GitHub's [actions-lock extension](https://github.com/github/gh-actions-lock).

After adding a workflow or changing an action reference, run from the repository
root with an authenticated GitHub CLI:

```sh
gh extension install github/gh-actions-lock --pin v0.4.1
gh actions-lock
gh actions-lock --verify
```

If the extension is already installed, use `gh extension upgrade gh-actions-lock`
to update it. Commit the workflow changes and generated lockfile together; never
edit the lockfile by hand. Review changed commits and investigate any changes to
`owner_id` or `repo_id`. Use `gh actions-lock --relock` to advance a branch pin
only after reviewing its upstream changes. `gh actions-lock --verify-local`
provides an offline coverage check.

The `actions-lock` job in `actions-lock.yml` verifies coverage and upstream provenance
on pushes and pull requests. Dependabot checks action versions weekly and updates
the matching lockfile entries for onboarded workflows.

## Token permissions

All workflows explicitly scope `GITHUB_TOKEN`; unspecified permissions are
disabled. The test and lockfile verification workflows grant only
`contents: read`. Other workflows default to `permissions: {}` and grant permissions to individual jobs:

| Jobs | Permissions | Purpose |
| --- | --- | --- |
| Tests and lockfile verification | `contents: read` | Checkout and dependency verification |
| Release builds | `contents: write` | Checkout, push tags, and manage releases and assets |
| CodeQL analysis | `contents: read`, `actions: read`, `security-events: write` | Checkout, workflow metadata, and code-scanning results |
| Stale issues and PRs | `issues: write`, `pull-requests: write`, `actions: write` | Label and close issues/PRs, and replace the saved state cache |

The stale action does not delete branches, so it does not need `contents: write`.

## CodeQL coverage

The CodeQL workflow scans C/C++ source and GitHub Actions workflows in separate
matrix jobs on pushes and pull requests. Both use the `security-extended` query
suite. C/C++ uses a manual build with the existing GTK setup; Actions uses
`build-mode: none` and skips the application setup and build steps. Results use
separate language categories in code scanning.

### Source suppressions

The C/C++ job also runs `codeql/cpp-queries:AlertSuppression.ql`. Place a
`// codeql[rule-id]` comment immediately before the flagged line and document
the justification alongside it. Comments apply only to the next line; the
rule remains enabled elsewhere.

The `advanced-security/dismiss-alerts` action, pinned through `actions.lock`, consumes the local
SARIF and dismisses matching alerts as "won't fix" only on pushes to the
repository's default branch. Pull requests and other branches generate
suppression metadata without changing repository-wide dismissals. Removing a
comment lets the action reopen alerts it previously dismissed; manual
dismissals are preserved. The action is recorded in `actions.lock`.

The three output-file permission suppressions intentionally retain the user's
umask for logs, debug text, and sector dumps. Newly created configuration files
exclude group and other write permissions.

Path-injection suppressions cover image, ECC, device, and log paths restored
from trusted user configuration. These settings intentionally accept arbitrary
paths, just like file selections made by the user; they are not confined to a
base directory. Configuration files must therefore be trusted by the user
running the application.

## Compatibility notes

The November 2026 update was checked against the upstream release notes:

- [Checkout v7](https://github.com/actions/checkout/tree/v7.0.1) restricts fork
  checkouts on `pull_request_target` and `workflow_run`; these workflows use
  `push` and `pull_request`. Its v6 credential storage change still supports
  authenticated `git push`, which the release jobs run on the host.
- [Stale v11](https://github.com/actions/stale/tree/v11.0.0) changes its internal
  module format without changing inputs, outputs, or behavior.
- [Release v3](https://github.com/softprops/action-gh-release/releases/tag/v3.0.0)
  moves to Node 24, already required by checkout v5 and stale v10 and supported
  by the GitHub-hosted runners used here.
- [CodeQL v4.39.3](https://github.com/github/codeql-action/blob/v4.39.3/CHANGELOG.md)
  supports the existing C++ analysis inputs. The workflow uses its default
  CodeQL bundle and checkout path, so the newer minimum bundle requirement does
  not require configuration changes.
- [MSYS2 v2.33.0](https://github.com/msys2/setup-msys2/releases/tag/v2.33.0)
  runs Windows builds in UCRT64 with matching `mingw-w64-ucrt-x86_64-*`
  packages. Packaging uses `MINGW_PREFIX` to locate the UCRT64 runtime DLLs
  and keeps the existing win64 archive names. The workflows do not rely on
  the removed default Perl installation.
- [Upload artifact v7.0.2](https://github.com/actions/upload-artifact/releases/tag/v7.0.2)
  preserves the screenshot upload inputs and matrix-specific artifact names.

The test workflow remains pull-request-only. Lockfile verification runs separately
on pushes and pull requests without duplicating the full regression matrix.
Release checkouts retain credentials because they push tags from the host;
test, CodeQL, and lockfile checkouts disable credential persistence.
