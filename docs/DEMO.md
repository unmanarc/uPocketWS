# Reproducing the screenshots

The images in [`screenshots/`](screenshots) were captured with a headless
Chrome/Chromium against a live `uPocketWS` instance serving a throw-away demo tree.
No mocked HTML is involved — what you see is what the server actually renders.

## 1. Prepare the demo tree

> Make sure the directory is **not** on a `noexec` mount if you also want to try
> the execution feature (`/tmp` is often mounted `noexec`).

```bash
DEMO="$HOME/tmp/uPocketWS-demo"

mkdir -p "$DEMO"/{docs,images,scripts,uploads}

printf 'Hello from uPocketWS! This is a plain text file.\n' > "$DEMO/hello.txt"
printf 'A demo text file to list in the index.\n'           > "$DEMO/demo-file.txt"
printf 'Rebuild notes for uPocketWS.\n'                     > "$DEMO/CHANGELOG.txt"
printf 'path: %s\nport: 8123\n' "$DEMO"                     > "$DEMO/config.yaml"
printf 'backup archive placeholder\n'                       > "$DEMO/backup.tar.gz"
printf 'image placeholder\n'                                > "$DEMO/images/logo.png"

printf '# uPocketWS\n\nThis is a demo documentation directory.\n' > "$DEMO/docs/README.md"
printf 'This is a demo page in the docs directory.\n'             > "$DEMO/docs/notes.txt"

printf '#!/bin/sh\necho "Hello from CGI-like script!"\necho "Date: $(date)"\n' \
  > "$DEMO/scripts/hello.sh"
chmod +x "$DEMO/scripts/hello.sh"
```

Do **not** place an `index.html` at the demo root: it would be served instead of the
generated directory listing, which is what we want to photograph.

## 2. Start the server

```bash
./build/uPocketWS \
  -r "$HOME/tmp/uPocketWS-demo" \
  -l 8123 \
  -u uploads -o uploads -m uploads -d uploads \
  -e scripts -g docs
```

Verify it answers:

```bash
curl -s -o /dev/null -w '%{http_code}\n' http://localhost:8123/
# 200
```

## 3. Capture

```bash
CHROME=/opt/google/chrome/google-chrome   # or chromium / chromium-browser
OUT=docs/screenshots
mkdir -p "$OUT"

shot() {
  "$CHROME" --headless=new --no-sandbox --disable-gpu \
            --hide-scrollbars --force-device-scale-factor=1 \
            --window-size=1440,900 \
            --screenshot="$OUT/$1.jpg" "$2"
}

shot 01-index        http://localhost:8123/
shot 02-docs-subdir  http://localhost:8123/docs/
shot 03-uploads      http://localhost:8123/uploads/
```

| File | What it shows |
| --- | --- |
| `01-index.jpg` | Root directory listing: folders first, file-type icons, size, mtime, UID:GID, mode |
| `02-docs-subdir.jpg` | Sub-directory with URL-derived breadcrumbs and the `Parent directory` entry |
| `03-uploads.jpg` | A directory with upload / overwrite / mkdir / delete enabled: drag & drop zone, `New folder` button |

`03-uploads.jpg` needs the `-u -o -m -d uploads` options, otherwise the upload zone
and the `New folder` button are not rendered at all.

## 4. Clean up

```bash
kill %1 2>/dev/null   # stop the server started above
rm -rf "$HOME/tmp/uPocketWS-demo"
```

## Notes

- The theme is embedded in the binary (`src/web/view/html/html_index_css.h`), so the
  captures do not depend on any external CSS/JS asset.
- The `Provided by Unmanarc's Pocket Web Server vX.Y.Z` footer comes straight from the
  compiled version, so refresh the screenshots whenever the version changes.
