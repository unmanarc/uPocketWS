#pragma once

// These snippets are inserted through "%s" arguments (never used as format strings).

// Toolbar button: download current directory as tar.gz
static constexpr const char *HTML_TARGZ_LINK = R"(            <a class="btn" href="./?action=targz" download><svg class="ic"><use href="#i-download"/></svg>Download .tar.gz</a>
)";

// Toolbar button: "New folder" (only included when mkdir is allowed in the current directory).
static constexpr const char *HTML_MKDIR_FORM = R"(            <button type="button" class="btn" id="mkdirbtn"><svg class="ic"><use href="#i-folder-plus"/></svg>New folder</button>
)";

// Upload drop zone (only included when uploads are allowed).
static constexpr const char *HTML_UPLOAD_FORM = R"(
        <form class="dropzone" id="dropzone" method="post" enctype="multipart/form-data">
            <svg class="ic" aria-hidden="true"><use href="#i-upload"/></svg>
            <div class="dz-text">
                <strong id="dz-title">Drag &amp; drop files here</strong>
                <span id="dz-sub">or choose them from your device</span>
            </div>
            <input type="file" id="file" name="file" multiple>
            <label for="file" class="btn">Choose files</label>
            <button type="submit" class="btn btn-primary" id="uploadbtn"><svg class="ic"><use href="#i-upload"/></svg>Upload</button>
            <progress id="upprogress" max="100" value="0" hidden></progress>
        </form>
)";

// Dialog: new folder (only included when mkdir is allowed).
static constexpr const char *HTML_DIALOG_MKDIR = R"(
<dialog id="dlg-mkdir">
    <form method="dialog">
        <h2>New folder</h2>
        <label for="mkdir-name" class="sr-only">Folder name</label>
        <input type="text" id="mkdir-name" class="field" placeholder="Folder name" required autocomplete="off">
        <div class="dialog-actions">
            <button type="button" class="btn" value="cancel" data-close>Cancel</button>
            <button type="submit" class="btn btn-primary" value="ok">Create</button>
        </div>
    </form>
</dialog>
)";

// Dialog: delete (only included when delete is allowed).
static constexpr const char *HTML_DIALOG_DELETE = R"(
<dialog id="dlg-delete">
    <form method="dialog">
        <h2>Delete</h2>
        <p id="del-msg"></p>
        <div class="dialog-actions">
            <button type="button" class="btn" value="cancel" data-close>Cancel</button>
            <button type="submit" class="btn btn-danger" value="ok">Delete</button>
        </div>
    </form>
</dialog>
)";

// Always included: shared helpers (CSRF, toasts, POST), breadcrumbs, file type icons,
// live filter and column sorting.
static constexpr const char *HTML_JS_COMMON = R"(
<script>
'use strict';
const getCookie = (name) => {
    const prefix = name + '=';
    for (const part of document.cookie.split(';')) {
        const c = part.trim();
        if (c.startsWith(prefix)) return decodeURIComponent(c.substring(prefix.length));
    }
    return '';
};

const toast = (msg, kind = 'info') => {
    const box = document.getElementById('toasts');
    const el = document.createElement('div');
    el.className = 'toast ' + kind;
    el.setAttribute('role', kind === 'err' ? 'alert' : 'status');
    el.textContent = msg;
    box.appendChild(el);
    setTimeout(() => el.remove(), 4000);
};

// POST to an action endpoint; the server answers "true" on success.
const postAction = async (url, body) => {
    try {
        const r = await fetch(url, { method: 'POST', headers: { 'X-CSRFToken': getCookie('CSRFToken') }, body });
        return r.ok && (await r.text()).trim() === 'true';
    } catch (e) {
        return false;
    }
};

const reloadSoon = () => setTimeout(() => location.reload(), 700);

const FILE_TYPES = {
    image: ['png', 'jpg', 'jpeg', 'gif', 'bmp', 'svg', 'webp', 'ico', 'tif', 'tiff', 'heic', 'avif'],
    video: ['mp4', 'mkv', 'avi', 'mov', 'webm', 'wmv', 'flv', 'm4v', 'mpg', 'mpeg'],
    audio: ['mp3', 'wav', 'flac', 'ogg', 'm4a', 'aac', 'opus', 'wma'],
    archive: ['zip', 'tar', 'gz', 'tgz', 'bz2', 'xz', 'zst', '7z', 'rar', 'rpm', 'deb', 'iso', 'jar'],
    code: ['c', 'h', 'cpp', 'hpp', 'cc', 'cxx', 'js', 'mjs', 'ts', 'py', 'rb', 'go', 'rs', 'java', 'kt', 'php', 'sh', 'bash',
           'pl', 'lua', 'html', 'htm', 'css', 'json', 'xml', 'yml', 'yaml', 'toml', 'sql', 'cmake', 'in', 'spec'],
    pdf: ['pdf'],
    text: ['txt', 'md', 'log', 'csv', 'ini', 'conf', 'cfg', 'rtf', 'doc', 'docx', 'odt']
};
const typeOf = (name) => {
    const dot = name.lastIndexOf('.');
    if (dot <= 0) return 'file';
    const ext = name.substring(dot + 1).toLowerCase();
    for (const [type, exts] of Object.entries(FILE_TYPES)) if (exts.includes(ext)) return type;
    return 'file';
};

document.addEventListener('DOMContentLoaded', () => {
    // Breadcrumbs from the current URL. The "/" root crumb is fixed in the
    // markup (first <li>, a link to "/"); only the remaining parts are
    // generated here, without the leading "/".
    const crumbs = document.getElementById('crumbs');
    const parts = location.pathname.split('/').filter(Boolean);
    crumbs.replaceChildren(crumbs.querySelector('li'));
    const decode = (p) => { try { return decodeURIComponent(p); } catch (e) { return p; } };
    let acc = '';
    parts.forEach((p, i, all) => {
        acc += '/' + p;
        const li = document.createElement('li');
        if (i === all.length - 1) {
            li.setAttribute('aria-current', 'page');
            li.textContent = decode(p);
        } else {
            const a = document.createElement('a');
            a.href = acc + '/';
            a.textContent = decode(p);
            li.appendChild(a);
        }
        crumbs.appendChild(li);
    });

    const tbody = document.querySelector('#listing tbody');
    const rows = Array.from(tbody.querySelectorAll('tr[data-name]'));

    // Per-type file icons
    rows.filter((tr) => tr.classList.contains('t-file')).forEach((tr) => {
        const t = typeOf(tr.dataset.name);
        if (t === 'file') return;
        tr.classList.replace('t-file', 't-' + t);
        tr.querySelector('.c-name use').setAttribute('href', '#i-' + t);
    });

    // Empty state
    const emptyRow = document.createElement('tr');
    emptyRow.className = 'empty';
    const emptyCell = document.createElement('td');
    emptyCell.colSpan = document.querySelectorAll('#listing thead th').length;
    emptyRow.appendChild(emptyCell);
    const refreshEmpty = () => {
        const visible = rows.some((r) => !r.hidden);
        emptyCell.textContent = rows.length ? 'No matching files' : 'This folder is empty';
        if (visible) emptyRow.remove(); else tbody.appendChild(emptyRow);
    };
    refreshEmpty();

    // Live filter
    const filter = document.getElementById('filter');
    filter.addEventListener('input', () => {
        const q = filter.value.trim().toLowerCase();
        rows.forEach((r) => { r.hidden = q !== '' && !r.dataset.name.toLowerCase().includes(q); });
        refreshEmpty();
    });

    // Column sorting (directories always first, parent row always on top)
    const collator = new Intl.Collator(undefined, { numeric: true, sensitivity: 'base' });
    document.querySelectorAll('#listing th[data-sort]').forEach((th) => {
        th.tabIndex = 0;
        const sortBy = () => {
            const key = th.dataset.sort;
            const dir = th.getAttribute('aria-sort') === 'ascending' ? -1 : 1;
            document.querySelectorAll('#listing th[aria-sort]').forEach((o) => o.removeAttribute('aria-sort'));
            th.setAttribute('aria-sort', dir === 1 ? 'ascending' : 'descending');
            rows.sort((a, b) => {
                const ad = a.classList.contains('t-dir'), bd = b.classList.contains('t-dir');
                if (ad !== bd) return ad ? -1 : 1;
                const v = key === 'name' ? collator.compare(a.dataset.name, b.dataset.name)
                                         : Number(a.dataset[key]) - Number(b.dataset[key]);
                return v * dir;
            });
            rows.forEach((r) => tbody.appendChild(r));
            refreshEmpty();
        };
        th.addEventListener('click', sortBy);
        th.addEventListener('keydown', (e) => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); sortBy(); } });
    });

    // Dialog cancel buttons
    document.querySelectorAll('dialog [data-close]').forEach((b) => b.addEventListener('click', () => b.closest('dialog').close('cancel')));
});
</script>
)";

// Upload handler: drag & drop + file picker, with progress (XMLHttpRequest exposes upload progress).
static constexpr const char *HTML_JS_UPLOAD = R"(
<script>
document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('dropzone');
    if (!form) return;
    const input = document.getElementById('file');
    const title = document.getElementById('dz-title');
    const sub = document.getElementById('dz-sub');
    const btn = document.getElementById('uploadbtn');
    const progress = document.getElementById('upprogress');
    let files = [];

    const fmtSize = (n) => {
        const u = ['B', 'kB', 'MB', 'GB', 'TB'];
        let i = 0;
        while (n >= 1024 && i < u.length - 1) { n /= 1024; i++; }
        return (i ? n.toFixed(1) : n) + ' ' + u[i];
    };
    const setFiles = (list) => {
        files = Array.from(list);
        if (!files.length) {
            title.textContent = 'Drag & drop files here';
            sub.textContent = 'or choose them from your device';
            return;
        }
        const total = files.reduce((s, f) => s + f.size, 0);
        title.textContent = files.length === 1 ? files[0].name : files.length + ' files selected';
        sub.textContent = fmtSize(total) + ' \u2014 ready to upload';
    };

    input.addEventListener('change', () => setFiles(input.files));

    ['dragenter', 'dragover'].forEach((ev) => form.addEventListener(ev, (e) => { e.preventDefault(); form.classList.add('dragover'); }));
    ['dragleave', 'drop'].forEach((ev) => form.addEventListener(ev, (e) => { e.preventDefault(); form.classList.remove('dragover'); }));
    form.addEventListener('drop', (e) => { if (e.dataTransfer && e.dataTransfer.files.length) setFiles(e.dataTransfer.files); });

    form.addEventListener('submit', (e) => {
        e.preventDefault();
        if (!files.length) { toast('Please select a file.', 'err'); return; }
        const fd = new FormData();
        files.forEach((f) => fd.append('file', f));

        const xhr = new XMLHttpRequest();
        xhr.open('POST', './?action=upload');
        xhr.setRequestHeader('X-CSRFToken', getCookie('CSRFToken'));
        btn.disabled = true;
        progress.hidden = false;
        progress.value = 0;
        xhr.upload.addEventListener('progress', (ev) => { if (ev.lengthComputable) progress.value = (ev.loaded / ev.total) * 100; });
        const done = (ok) => {
            btn.disabled = false;
            progress.hidden = true;
            if (ok) { toast('File(s) uploaded', 'ok'); reloadSoon(); }
            else toast('File(s) not uploaded', 'err');
        };
        xhr.addEventListener('load', () => done(xhr.status >= 200 && xhr.status < 300 && xhr.responseText.trim() === 'true'));
        xhr.addEventListener('error', () => done(false));
        xhr.addEventListener('abort', () => done(false));
        xhr.send(fd);
    });
});
</script>
)";

// Delete handler (only included when deletion is allowed in the current directory).
// Delegated click on .btn-del buttons, confirmed with a native <dialog>.
static constexpr const char *HTML_JS_DELETE = R"(
<script>
document.addEventListener('DOMContentLoaded', () => {
    const dlg = document.getElementById('dlg-delete');
    const msg = document.getElementById('del-msg');
    let target = null;

    document.getElementById('listing').addEventListener('click', (e) => {
        const b = e.target.closest('.btn-del');
        if (!b) return;
        target = b;
        msg.textContent = b.dataset.dir === '1'
            ? 'Delete directory "' + b.dataset.name + '" and ALL its contents? This cannot be undone.'
            : 'Delete file "' + b.dataset.name + '"? This cannot be undone.';
        dlg.returnValue = '';
        dlg.showModal();
    });

    dlg.addEventListener('close', async () => {
        if (dlg.returnValue !== 'ok' || !target) return;
        const b = target;
        target = null;
        b.disabled = true;
        if (await postAction(b.dataset.href)) {
            toast('Deleted "' + b.dataset.name + '"', 'ok');
            reloadSoon();
        } else {
            b.disabled = false;
            toast('Delete failed', 'err');
        }
    });
});
</script>
)";

// New directory handler (only included when mkdir is allowed in the current directory).
// Asks for the name with a native <dialog>, then POSTs it to the current
// directory as ?action=mkdir&name=<name>.
static constexpr const char *HTML_JS_MKDIR = R"(
<script>
document.addEventListener('DOMContentLoaded', () => {
    const btn = document.getElementById('mkdirbtn');
    const dlg = document.getElementById('dlg-mkdir');
    if (!btn || !dlg) return;
    const input = document.getElementById('mkdir-name');

    btn.addEventListener('click', () => {
        input.value = '';
        dlg.returnValue = '';
        dlg.showModal();
        input.focus();
    });

    dlg.addEventListener('close', async () => {
        if (dlg.returnValue !== 'ok') return;
        const name = input.value.trim();
        if (name === '') { toast('Name is empty.', 'err'); return; }
        if (await postAction('./?action=mkdir&name=' + encodeURIComponent(name))) {
            toast('Directory created', 'ok');
            reloadSoon();
        } else {
            toast('Directory not created', 'err');
        }
    });
});
</script>
)";
