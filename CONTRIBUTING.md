# Contributing to uPocketWS

Thanks for taking the time to improve `uPocketWS`. Bug reports, patches, packaging
fixes and documentation updates are all welcome. This file explains what the
project expects so a contribution can be reviewed quickly.

Back to [`README.md`](README.md) · [`INSTALL.md`](INSTALL.md) · [`BUILD.md`](BUILD.md)

---

## Scope rules

`uPocketWS` deliberately stays a single small, dependency-free C++ binary. Before
writing code, check that the change respects these constraints:

- **No new runtime dependency.** The stack is fixed: libMantids30, jsoncpp, Boost
  (`regex`, `thread`) and — only for TLS builds — OpenSSL + zlib.
- **No configuration file.** Everything is a command line switch with a sane
  default.
- **New capabilities are opt-in and directory scoped.** Anything that touches the
  filesystem beyond plain reads must be gated behind a per-directory allow-list
  (`--execute-dir`, `--archive-dir`, `--upload-dir`, `--overwrite-dir`,
  `--delete-dir`, `--mkdir-dir`), consistent with the existing ones.
- **Small, focused changes.** One topic per pull request; avoid drive-by rewrites.

If a feature needs a new library or a config file, open an issue first — it most
likely belongs in a different project.

---

## Reporting bugs

Use the [bug report template](.github/ISSUE_TEMPLATE/bug_report.md) (GitHub shows
it when you open an issue). A useful report includes:

- `uPocketWS -h` first line (exact version).
- Distribution and version (e.g. RHEL 10.0, Rocky 9.5, Fedora 42, Ubuntu 24.04).
- libMantids30 version and whether the binary was built with
  `-DWITH_SSL_SUPPORT=ON`.
- How it was installed (COPR package / built from source, and the prefix).
- The exact command line, with credentials and certificate paths redacted.
- The output of `--debugparams` when the program gets that far, plus the relevant
  server log (`journalctl -u uPocketWS` when running with `--sys`).
- Which optional capabilities were enabled (`--passfile`, TLS, `--execute-dir`,
  `--archive-dir`, upload/overwrite, delete/mkdir, `--daemon`).

---

## Requesting features

Use the [feature request template](.github/ISSUE_TEMPLATE/feature_request.md) and
describe the problem before the solution. For a new switch, propose the long and
short option names and the default value, matching the existing naming style
(`--upload-name-re`, `--mkdir-dir`, ...).

---

## Development setup

Clone and build as described in [`BUILD.md`](BUILD.md). The short version:

```bash
git clone https://github.com/unmanarc/uPocketWS
cd uPocketWS
cmake -DCMAKE_BUILD_TYPE=MinSizeRel -B../uPocketWS-build .
cmake --build ../uPocketWS-build -j"$(nproc)"
```

Because TLS support changes the argument parser and the socket layer, keep a
second build tree for it:

```bash
cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DWITH_SSL_SUPPORT=ON -B../uPocketWS-build-tls .
cmake --build ../uPocketWS-build-tls -j"$(nproc)"
```

## Smoke test before submitting

Run the server against a throwaway directory and exercise the code path you
touched. Enable only the capabilities you need:

```bash
mkdir -p /tmp/share/downloads /tmp/share/incoming
../uPocketWS-build/uPocketWS -r /tmp/share -l 8001 \
  -g . -u . -o . -d . -m . -n '^[A-Za-z0-9._-]+$'
```

```bash
# Directory index
curl -sS -o /dev/null -w '%{http_code}\n' http://127.0.0.1:8001/

# On-the-fly archive (needs --archive-dir)
curl -sS -o /tmp/share.tar.gz 'http://127.0.0.1:8001/?action=targz'
tar -tzf /tmp/share.tar.gz | head

# Mutating actions are POSTs protected by a double-submit CSRF token:
# the GET below sets the CSRFToken cookie, and every POST must echo it in
# the X-CSRFToken header (this is what the web UI does).
curl -sS -c /tmp/upws.cookies -o /dev/null http://127.0.0.1:8001/
CSRF=$(awk '$6 == "CSRFToken" { print $7 }' /tmp/upws.cookies)

# mkdir / upload / delete (need --mkdir-dir / --upload-dir / --delete-dir)
curl -sS -b /tmp/upws.cookies -H "X-CSRFToken: $CSRF" \
     -X POST 'http://127.0.0.1:8001/?action=mkdir&name=ci-test'
curl -sS -b /tmp/upws.cookies -H "X-CSRFToken: $CSRF" \
     -X POST -F 'file=@/etc/hostname' 'http://127.0.0.1:8001/?action=upload'
curl -sS -b /tmp/upws.cookies -H "X-CSRFToken: $CSRF" \
     -X POST 'http://127.0.0.1:8001/?action=delete&file=hostname'
```

A successful `POST` answers `true`; a missing or mismatched token answers `401`
and logs `R/POST-FAIL-CSRF`.

Also check the browser UI when you change anything under `src/web/view/`, and
include before/after screenshots (1440x900) in the pull request — the capture
procedure is documented in [`docs/DEMO.md`](docs/DEMO.md).
---

## Coding conventions

Match the surrounding code — consistency beats personal preference:

- **C++17, no compiler extensions** (`CMAKE_CXX_EXTENSIONS OFF`). Do not use features
  that require C++20; RHEL 8 builds with gcc 8.
- **4 spaces** for indentation (no tabs) and **Allman braces** (opening brace on its
  own line), as in `src/main.cpp` and `src/web/`.
- **`#pragma once`** in headers; existing ones all use it.
- **Namespaces:** `uPocketWS::Web` for the HTTP layer; `using namespace Mantids30;`
  only in translation units that already do so.
- **Naming:** `CamelCase` for types, `camelCase` for members and locals
  (`dirInfo.fullPath`), `snake_case` only where libMantids30 forces it.
- **Filesystem access:** use `std::filesystem` (aliased as `namespace fs = ...`)
  and validate every request-derived path with `fs::canonical()` against the
  configured root before touching it.
- **Portability:** keep the `_WIN32` guards in `src/main.cpp` compiling, and do not
  assume `std::filesystem` lives in libstdc++ (gcc < 9 links `-lstdc++fs`, which
  `CMakeLists.txt` adds automatically).
- **Logging:** go through the existing `AppLog`/`weblog` facilities so `--sys`
  (journald, no colors or dates) keeps working.

## Pull requests

1. Fork, create a topic branch, and keep the diff limited to that topic.
2. Describe the change in one or two sentences and link the issue it closes.
3. Tick the checklists in
   [`.github/PULL_REQUEST_TEMPLATE.md`](.github/PULL_REQUEST_TEMPLATE.md). The
   security checklist is not optional — this program serves files:
   - new filesystem access is canonicalised and stays inside `--rootdir`
     (no `..` traversal, no symlink escape);
   - new capabilities sit behind a per-directory allow-list;
   - no new runtime dependency;
   - no request-derived path is passed to a shell.
4. Confirm the build has **no new warnings**, both with and without
   `-DWITH_SSL_SUPPORT=ON`, and state which distribution(s) you tested on.
5. Update the documentation when user-visible behavior changes:
   [`README.md`](README.md) for features, [`INSTALL.md`](INSTALL.md) for flags and
   operation, [`BUILD.md`](BUILD.md) for the build system.

## Packaging

RPM packaging lives in [`rpm/build.spec`](rpm/build.spec) and is built through COPR
(`amizrachi/unmanarc`). Changes there must keep the RHEL 7 (`yum`, `cmake3`,
`openssl11`) through RHEL 10 (`dnf5`) conditionals intact, and add new shipped
documents to the `%doc` list.

## License of contributions

`uPocketWS` is distributed under the **GNU Affero General Public License v3.0**
(see [`LICENSE`](LICENSE)). By submitting a pull request you agree to license your
contribution under the same terms (inbound = outbound). Do not paste code from
projects with an incompatible license.
