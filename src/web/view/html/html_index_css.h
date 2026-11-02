#pragma once

// Light theme stylesheet for the directory index page.
// Inserted through a "%s" argument, so '%' characters here are safe.
static constexpr const char *HTML_CSS_PAGE = R"(
<style>
:root {
    --bg: #f5f7fb;
    --surface: #ffffff;
    --surface-alt: #f9fafc;
    --border: #e3e7ef;
    --text: #1f2937;
    --muted: #6b7280;
    --primary: #2563eb;
    --primary-hover: #1d4ed8;
    --primary-soft: #eff4ff;
    --danger: #dc2626;
    --danger-soft: #fef2f2;
    --success: #16a34a;
    --radius: 10px;
    --shadow: 0 1px 2px rgba(16, 24, 40, .05), 0 4px 16px rgba(16, 24, 40, .06);
    --mono: ui-monospace, SFMono-Regular, "JetBrains Mono", Menlo, Consolas, "Liberation Mono", monospace;
    color-scheme: light;
}
*, *::before, *::after { box-sizing: border-box; }
body {
    margin: 0;
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    background: var(--bg);
    color: var(--text);
    font: 14px/1.5 system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
}
a { color: var(--primary); text-decoration: none; }
a:hover { color: var(--primary-hover); text-decoration: underline; }
.wrap { width: 100%; max-width: 1180px; margin: 0 auto; padding: 0 20px; }
.sr-only { position: absolute; width: 1px; height: 1px; overflow: hidden; clip: rect(0 0 0 0); white-space: nowrap; }
[hidden] { display: none !important; }

/* Icons */
.ic { width: 18px; height: 18px; flex: none; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; vertical-align: middle; }

/* Header */
.app-header { background: var(--surface); border-bottom: 1px solid var(--border); }
.app-header .wrap { display: flex; align-items: center; gap: 14px; padding-top: 16px; padding-bottom: 16px; }
.brand { display: flex; align-items: center; justify-content: center; width: 40px; height: 40px; border-radius: 10px; background: var(--primary); color: #fff; flex: none; }
.brand .ic { width: 22px; height: 22px; }
.title-block { min-width: 0; }
.title-block h1 { margin: 0; font-size: 18px; font-weight: 600; }
.crumbs ol { display: flex; flex-wrap: wrap; align-items: center; margin: 2px 0 0; padding: 0; list-style: none; font-family: var(--mono); font-size: 13px; color: var(--muted); }
.crumbs li + li::before { content: "/"; margin: 0 6px; color: #c0c6d2; }
.crumbs li.crumb-root + li::before { content: none; }
.crumbs li.crumb-root:not(:only-child) { margin-right: 6px; }
.crumbs a { color: var(--muted); }
.crumbs a:hover { color: var(--primary); }
.crumbs [aria-current] { color: var(--text); font-weight: 600; }

/* Main */
main { flex: 1; padding: 24px 0 40px; }
.toolbar { display: flex; flex-wrap: wrap; align-items: center; gap: 10px; margin-bottom: 16px; }
.toolbar .spacer { flex: 1; }
.search { position: relative; display: flex; align-items: center; }
.search .ic { position: absolute; left: 10px; color: var(--muted); width: 16px; height: 16px; pointer-events: none; }
.search input { width: 260px; max-width: 100%; padding: 8px 12px 8px 34px; border: 1px solid var(--border); border-radius: 8px; background: var(--surface); font: inherit; color: inherit; }
.search input:focus, .field:focus { outline: none; border-color: var(--primary); box-shadow: 0 0 0 3px rgba(37, 99, 235, .15); }

.btn { display: inline-flex; align-items: center; gap: 8px; padding: 8px 14px; border: 1px solid var(--border); border-radius: 8px; background: var(--surface); color: var(--text); font: inherit; font-weight: 500; cursor: pointer; text-decoration: none; transition: background .15s, border-color .15s, color .15s; }
.btn:hover { background: var(--surface-alt); border-color: #cfd5e1; color: var(--text); text-decoration: none; }
.btn .ic { width: 16px; height: 16px; }
.btn-primary { background: var(--primary); border-color: var(--primary); color: #fff; }
.btn-primary:hover { background: var(--primary-hover); border-color: var(--primary-hover); color: #fff; }
.btn-danger { background: var(--danger); border-color: var(--danger); color: #fff; }
.btn-danger:hover { background: #b91c1c; border-color: #b91c1c; color: #fff; }
.btn:disabled { opacity: .6; cursor: not-allowed; }
.btn-icon { display: inline-flex; align-items: center; justify-content: center; width: 32px; height: 32px; padding: 0; border: 1px solid transparent; border-radius: 8px; background: transparent; color: var(--muted); cursor: pointer; }
.btn-icon .ic { width: 16px; height: 16px; }
.btn-del:hover { color: var(--danger); background: var(--danger-soft); border-color: #fecaca; }

/* Upload drop zone */
.dropzone { display: flex; flex-wrap: wrap; align-items: center; gap: 14px; margin-bottom: 16px; padding: 18px 20px; border: 2px dashed #cdd5e3; border-radius: var(--radius); background: var(--surface); transition: border-color .15s, background .15s; }
.dropzone.dragover { border-color: var(--primary); background: var(--primary-soft); }
.dropzone > .ic { width: 28px; height: 28px; color: var(--primary); }
.dz-text { flex: 1; min-width: 200px; }
.dz-text strong { display: block; font-weight: 600; }
.dz-text span { color: var(--muted); font-size: 13px; }
.dropzone input[type=file] { display: none; }
.dropzone progress { width: 100%; height: 6px; border: 0; border-radius: 3px; overflow: hidden; appearance: none; background: #e5e9f2; }
.dropzone progress::-webkit-progress-bar { background: #e5e9f2; }
.dropzone progress::-webkit-progress-value { background: var(--primary); }
.dropzone progress::-moz-progress-bar { background: var(--primary); }

/* Listing table */
.card { background: var(--surface); border: 1px solid var(--border); border-radius: var(--radius); box-shadow: var(--shadow); overflow: hidden; }
.listing { width: 100%; border-collapse: collapse; }
.listing th, .listing td { padding: 10px 16px; text-align: left; white-space: nowrap; }
.listing thead th { background: var(--surface-alt); border-bottom: 1px solid var(--border); color: var(--muted); font-size: 12px; font-weight: 600; letter-spacing: .04em; text-transform: uppercase; user-select: none; }
.listing thead th[data-sort] { cursor: pointer; }
.listing thead th[data-sort]:hover { color: var(--text); }
.listing thead th[aria-sort=ascending]::after { content: " \25B2"; font-size: 9px; }
.listing thead th[aria-sort=descending]::after { content: " \25BC"; font-size: 9px; }
.listing tbody tr { border-bottom: 1px solid var(--border); transition: background .1s; }
.listing tbody tr:last-child { border-bottom: 0; }
.listing tbody tr:hover { background: var(--primary-soft); }
.c-name { width: 100%; max-width: 0; }
.c-name a { display: flex; align-items: center; gap: 10px; color: var(--text); overflow: hidden; }
.c-name a span { overflow: hidden; text-overflow: ellipsis; }
.c-name a:hover { color: var(--primary); text-decoration: none; }
.c-size, .c-owner, .c-perm, .c-mtime { color: var(--muted); font-family: var(--mono); font-size: 13px; }
.listing .c-size { text-align: right; }
.listing .c-act { width: 1%; text-align: center; padding-top: 4px; padding-bottom: 4px; }
.perm-warn { color: var(--danger); font-weight: 600; cursor: help; }
.empty td { padding: 40px 16px; text-align: center; color: var(--muted); white-space: normal; }

/* File type icon colors */
.t-dir .ic, .t-up .ic { color: #f59e0b; }
.t-dir .ic { fill: #fde68a; }
.t-dir .c-name a span { font-weight: 500; }
.t-file .ic { color: #64748b; }
.t-image .ic { color: #8b5cf6; }
.t-video .ic { color: #ec4899; }
.t-audio .ic { color: #06b6d4; }
.t-archive .ic { color: #b45309; }
.t-code .ic { color: #16a34a; }
.t-pdf .ic { color: #dc2626; }
.t-text .ic { color: #2563eb; }

.alert { padding: 24px; border: 1px solid #fecaca; border-radius: var(--radius); background: var(--danger-soft); color: var(--danger); font-weight: 500; }

/* Footer */
.app-footer { border-top: 1px solid var(--border); background: var(--surface); color: var(--muted); font-size: 13px; }
.app-footer .wrap { padding-top: 14px; padding-bottom: 14px; text-align: center; }

/* Dialog */
dialog { width: min(420px, calc(100vw - 32px)); padding: 0; border: 1px solid var(--border); border-radius: 12px; box-shadow: 0 20px 50px rgba(16, 24, 40, .2); color: var(--text); }
dialog::backdrop { background: rgba(15, 23, 42, .35); backdrop-filter: blur(2px); }
dialog form { padding: 20px; }
dialog h2 { margin: 0 0 8px; font-size: 16px; }
dialog p { margin: 0 0 14px; color: var(--muted); word-break: break-word; }
.field { width: 100%; margin-bottom: 16px; padding: 9px 12px; border: 1px solid var(--border); border-radius: 8px; font: inherit; color: inherit; }
.dialog-actions { display: flex; justify-content: flex-end; gap: 8px; }

/* Toasts */
.toasts { position: fixed; right: 20px; bottom: 20px; z-index: 100; display: flex; flex-direction: column; gap: 8px; }
.toast { min-width: 240px; max-width: 380px; padding: 12px 16px; border-left: 4px solid var(--primary); border-radius: 8px; background: var(--surface); box-shadow: var(--shadow); animation: toast-in .2s ease-out; }
.toast.ok { border-left-color: var(--success); }
.toast.err { border-left-color: var(--danger); }
@keyframes toast-in { from { opacity: 0; transform: translateY(8px); } to { opacity: 1; transform: none; } }

@media (max-width: 760px) {
    .c-owner, .c-perm { display: none; }
    .search, .search input { width: 100%; }
    .toolbar .spacer { display: none; }
}
@media (max-width: 520px) {
    .c-mtime { display: none; }
    .listing th, .listing td { padding: 10px; }
}
</style>
)";
