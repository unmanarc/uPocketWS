## What does this PR do?

<!-- One or two sentences. Link the issue it closes, e.g. "Closes #12". -->

## Type of change

- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (existing command line behavior changes)
- [ ] Documentation / packaging only (no `src/` change)

## Affected areas

- [ ] `src/main.cpp` — argument parsing / startup
- [ ] `src/web/` — HTTP dispatch, handlers, views
- [ ] `CMakeLists.txt` / build system
- [ ] `rpm/build.spec` / packaging
- [ ] `README.md` / `docs/`
- [ ] `.github/`

## Security checklist

The server exposes files, so please confirm:

- [ ] New filesystem access is validated with `fs::canonical()` and stays inside
      `--rootdir` (no `..` traversal, no symlink escape)
- [ ] New capabilities are gated behind a per-directory allow-list option
      (`--execute-dir`, `--archive-dir`, `--upload-dir`, `--overwrite-dir`,
      `--delete-dir`, `--mkdir-dir`)
- [ ] No new runtime dependency was introduced
- [ ] Paths coming from the request are not passed to a shell

## How was this tested?

<!-- Commands used, distributions, TLS on/off, manual browser checks... -->

```bash
cmake -DCMAKE_BUILD_TYPE=MinSizeRel -B../uPocketWS-build .
cmake --build ../uPocketWS-build -j"$(nproc)"
```

- [ ] Builds without new warnings
- [ ] Verified manually with `curl` / a browser
- [ ] Checked both with and without `-DWITH_SSL_SUPPORT=ON`

## Screenshots (for web UI changes)

<!-- Before / after, 1440x900, see docs/DEMO.md for the capture procedure -->
