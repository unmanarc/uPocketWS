# Installing and running uPocketWS

How to get `uPocketWS` onto a machine (prebuilt COPR packages or a manual
build), how to drive it from the command line, how to run it under systemd and
which security rules to respect.

Back to [`README.md`](README.md) for the feature overview, or to
[`BUILD.md`](BUILD.md) for the full build instructions.

---

## COPR — Fedora / RHEL / CentOS / Rocky / AlmaLinux

```bash
# Fedora
dnf -y install dnf-plugins-core
dnf copr enable amizrachi/unmanarc -y
dnf -y install uPocketWS
```

```bash
# RHEL 10 / Rocky 10 / AlmaLinux 10  (dnf5)
dnf -y install dnf5-plugins epel-release
dnf -y config-manager setopt crb.enabled=1
dnf copr enable amizrachi/unmanarc -y
dnf -y install uPocketWS
```

```bash
# RHEL 9 / Rocky 9 / AlmaLinux 9  (dnf4)
dnf -y install dnf-plugins-core epel-release
dnf -y config-manager --set-enabled crb
dnf copr enable amizrachi/unmanarc -y
dnf -y install uPocketWS
```

```bash
# RHEL 8 / CentOS 8 Stream
dnf -y install 'dnf-command(config-manager)' epel-release
dnf config-manager --set-enabled powertools
dnf copr enable amizrachi/unmanarc -y
dnf -y install uPocketWS
```

```bash
# RHEL 7 / CentOS 7 (EOL, yum + cmake3)
yum -y install epel-release yum-plugin-copr
yum copr enable amizrachi/unmanarc -y
yum -y install uPocketWS
```

> **RHEL 10 note:** it ships `dnf5`, so the plugin package is `dnf5-plugins` and
> `config-manager` uses the `setopt` sub-command instead of the `dnf4`-style
> `--set-enabled` flag. Skip the `crb` line if your tree does not use CRB.

---

## Debian / Ubuntu — build from source

There is no APT repository for `uPocketWS`, so on Debian and Ubuntu the
recommended path is building from source. Install the toolchain and libraries
first:

```bash
sudo apt update
sudo apt -y install build-essential cmake pkg-config git \
                    libboost-regex-dev libboost-thread-dev \
                    zlib1g-dev libssl-dev
```

`libMantids30` is not packaged in Debian/Ubuntu either — build and install it
from <https://github.com/unmanarc/libMantids30> before configuring `uPocketWS`,
or point `-DMANTIDS30_PREFIX=` at the prefix it was installed into.

Then follow [`BUILD.md`](BUILD.md) for the CMake invocation.

---
## Quick start

Serve the current directory on the default port `8001`:

```bash
uPocketWS
```

Then open <http://localhost:8001/>.

Ports below 1024 (`-l 80`) need extra privileges on Linux: run as root, grant
`CAP_NET_BIND_SERVICE`, or redirect with a firewall/nftables rule.

A more realistic example — read-only share with authentication, on-the-fly archives
and a dedicated directory where uploads/overwrite/delete/mkdir are allowed:

```bash
uPocketWS \
  -r /srv/files \
  -l 8080 \
  -p /etc/uPocketWS/users.conf \
  -g downloads \
  -u incoming -o incoming -d incoming -m incoming -n '^[A-Za-z0-9._-]+$'
```

---

## Command line reference

```text
$ uPocketWS -h
# Unmanarc's Pocket Web Server (uPocketWS) v3.0.0 a
# Author:  Aarón Mizrachi (aaron@unmanarc.com)
# License: AGPL

Help:
-----

HTTP Options:
-------------
-r --rootdir <value> : HTTP Document Root Directory (default: .)
-l --lport   <value> : Local HTTP Port (default: 8001)
-4 --ipv4            : Use only IPv4 (default: true)
-a --laddr   <value> : Listen Address (default: *)
-t --threads <value> : Max Concurrent Connections (Threads) (default: 1024)

HTTP Security:
--------------
-p --passfile <value> : HTTP User/Pass File (default: )

Other Options:
--------------
   --debugparams         : Debug parameters and exit. (default: false)
   --verbose     <value> : Set verbosity level. (default: 0)
-h --help                : Show information usage. (default: false)
-s --sys                 : Journalctl Log Mode (don't print colors or dates) (default: false)

Server Options:
---------------
-e --execute-dir    <value> : Execute any eXecutable marked file in this directory (relative to --rootdir, repeatable, sub-dirs allowed) and get the output, ATT: avoid enabling it in directories you also allow to archive (default: )
-g --archive-dir    <value> : Allow to get this directory (relative to --rootdir, repeatable, sub-dirs allowed) as a tar.gz file (on-the-fly) (default: )
-u --upload-dir     <value> : Allow file uploads (creating new files) to this directory (relative to --rootdir, repeatable, sub-dirs allowed) (default: )
-n --upload-name-re <value> : Regex the uploaded filename must fully match (if set, non-matching names are rejected) (default: )
-o --overwrite-dir  <value> : Allow overwriting existing files on upload in this directory (relative to --rootdir, repeatable, sub-dirs allowed) (default: )
-d --delete-dir     <value> : Allow deleting files/directories in this directory (relative to --rootdir, repeatable, sub-dirs allowed) (default: )
-m --mkdir-dir      <value> : Allow creating new directories in this directory (relative to --rootdir, repeatable, sub-dirs allowed) (default: )

Service Options:
----------------
   --daemon            : Run as daemon. (default: false)
   --install   <value> : Install this program with systemd file/service name (default: )
   --reinstall         : Reinstall this program if it's already installed (default: false)
   --uninstall <value> : Uninstall this program with systemd file/service name (default: )
```

When the program is compiled with `-DWITH_SSL_SUPPORT=ON`, an extra section is
printed:

```text
TLS Options:
------------
   --cafile   <value> : X.509 Certificate Authority Path (if not defined, then without client authentication) (default: )
-k --keyfile  <value> : X.509 Private Key Path (if not defined, then HTTP) (default: )
-c --certfile <value> : X.509 Certificate Path (if not defined, then HTTP) (default: )
```

`--execute-dir`, `--archive-dir`, `--upload-dir`, `--overwrite-dir`,
`--delete-dir` and `--mkdir-dir` are **repeatable** and accept paths relative to
`--rootdir`, sub-directories included.

---

## Running as a service

Create the unit by hand:

```bash
cat << 'UNIT' | install -m 640 /dev/stdin /usr/lib/systemd/system/uPocketWS.service
[Unit]
Description=Unmanarc's Pocket Web Server
After=network.target

[Service]
Type=simple
Restart=always
RestartSec=1
ExecStart=/opt/osslibs/bin/uPocketWS -s -r /isos
#User=
#Group=

[Install]
WantedBy=multi-user.target
UNIT

systemctl daemon-reload
systemctl enable --now uPocketWS.service
```

> Change the document root (`/isos` above) and set `User=`/`Group=` for your setup.

Or let the binary write the unit for you (and manage it later):

```bash
sudo uPocketWS --install uPocketWS     # write the systemd unit
sudo uPocketWS --reinstall             # rewrite an existing unit
sudo uPocketWS --uninstall uPocketWS   # remove it again
```

---

## Security considerations

1. **Use encryption.** Plain HTTP can be eavesdropped or tampered with. Put the
   server behind a VPN/Tor, or build with `-DWITH_SSL_SUPPORT=ON` (see
   [`BUILD.md`](BUILD.md)) and use certificates signed by a trusted CA.
2. **Scope every capability.** Each feature is bound to the directories given with
   `--execute-dir`, `--archive-dir`, `--upload-dir`, `--overwrite-dir`,
   `--delete-dir` and `--mkdir-dir`. Do not combine `--execute-dir` with
   `--archive-dir` on the same tree: the executables themselves could be downloaded,
   leaking hardcoded credentials.
3. **Every executable in an `--execute-dir` is callable.** Audit the contents of
   those directories, validate the input the scripts receive, and make sure it cannot
   be used for injection in other programs.
4. **Never run as root**, especially with execution mode enabled.
5. **Passfile format:** first line is the user name, second line the password in
   clear text. Protect the file with `chmod 600`.
6. **Uploads** (new files) are only accepted inside `--upload-dir` trees; replacing
   an existing file additionally needs `--overwrite-dir`; deletion and `mkdir` need
   `--delete-dir` and `--mkdir-dir`. Use `--upload-name-re` to restrict accepted file
   names with a full-match regular expression.
