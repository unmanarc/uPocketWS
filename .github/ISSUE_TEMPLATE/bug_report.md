---
name: Bug report
about: Report a defect so it can be reproduced and fixed
title: "[BUG] "
labels: bug
assignees: unmanarc
---

## Description

A clear and concise description of the problem.

## Steps to reproduce

1.
2.
3.

## Expected behavior

What you expected to happen.

## Actual behavior

What actually happened. Include error output or a log excerpt if available.

## Command line used

Paste the exact invocation, with secrets/certificates paths redacted.

```bash
uPocketWS -r /path/to/root -l 8001 ...
```

Output of `uPocketWS --debugparams` (if it runs that far):

```text

```

## Environment

- uPocketWS version (`uPocketWS -h` first line):
- Distribution / version (e.g. RHEL 10.0, Rocky 9.5, Fedora 42, Ubuntu 24.04):
- libMantids30 version:
- Built with TLS support (`-DWITH_SSL_SUPPORT=ON`)? yes / no
- How installed: COPR package / built from source

## Relevant options in use

- [ ] `--passfile` (authentication)
- [ ] TLS (`--keyfile` / `--certfile` / `--cafile`)
- [ ] `--execute-dir`
- [ ] `--archive-dir`
- [ ] `--upload-dir` / `--overwrite-dir`
- [ ] `--delete-dir` / `--mkdir-dir`
- [ ] `--daemon` / systemd unit

## Additional context

Screenshots, packet captures, strace output, anything else that helps.
