#pragma once

static constexpr const char *HTML_TARGZ_LINK = R"(<tr class='tableupload'>
     <td class='tableupload' align=left> -> <a href="./?action=targz">Download dir as tar.gz</a>
     </td>
</tr>)";

static constexpr const char *HTML_UPLOAD_FORM = R"(
    <form method='post' enctype='multipart/form-data'>
        <table align='center' width='80%' class='tableupload'>
            <tr width='100%' class='tableupload'>
                <td align=left class='tableupload'>
                    <label for='file'>Choose file to upload -> </label>
                    <input type='file' id='file' name='file' multiple>
                </td>
                <td align=right class='tableupload'>
                <input type='button' class='button' value='Upload' id='uploadbtn'>
                </td>
            </tr>
        </table>
    </form>
)";

// "New dir" button (only included when can-overwrite/create is allowed).
static constexpr const char *HTML_MKDIR_FORM = R"(
    <tr class='tableupload'>
        <td class='tableupload' align=left>
            <input type='button' class='button' value='+ New dir' id='mkdirbtn'>
        </td>
    </tr>
)";

// Always included: shared helpers (CSRF cookie reader).
// https://www.w3schools.com/js/js_cookies.asp
static constexpr const char *HTML_JS_COMMON = R"(
<script>
// From: https://www.w3schools.com/js/js_cookies.asp
function getCookie(cname) {
  let name = cname + '=';
  let decodedCookie = decodeURIComponent(document.cookie);
  let ca = decodedCookie.split(';');
  for(let i = 0; i <ca.length; i++) {
    let c = ca[i];
    while (c.charAt(0) == ' ') {
      c = c.substring(1);
    }
    if (c.indexOf(name) == 0) {
      return c.substring(name.length, c.length);
    }
  }
  return '';
}
</script>
)";

static constexpr const char *HTML_JS_UPLOAD = R"(
<script>
document.addEventListener('DOMContentLoaded', function(){
    document.getElementById('uploadbtn').addEventListener('click', function(){
        var fd = new FormData();
        var files = document.getElementById('file').files;

        // Check file selected or not
        if(files.length == 0 ){
            alert('Please select a file.'); return;
        }
        for (var x = 0; x < files.length; x++) {
            fd.append('file', files[x]);
        }
        fetch('./.upload', {
            method: 'POST',
            headers: { 'X-CSRFToken': getCookie('CSRFToken') },
            body: fd
        })
        .then(function(response){
            if(!response.ok) { throw new Error('HTTP ' + response.status); }
            return response.text();
        })
        .then(function(response){
            if(response.trim() == 'true') { alert('File(s) uploaded'); location.reload(); }
            else{ alert('File(s) not uploaded'); }
        })
        .catch(function(){
            alert('File(s) not uploaded');
        });
    });
});
</script>
)";

// Delete handler (only included when can-overwrite/delete is allowed).
static constexpr const char *HTML_JS_DELETE = R"(
<script>
function deleteEntry(a) {
    fetch(a.href, {
        method: 'POST',
        headers: { 'X-CSRFToken': getCookie('CSRFToken') }
    })
    .then(function(response){
        if(!response.ok) { throw new Error('HTTP ' + response.status); }
        return response.text();
    })
    .then(function(response){
        if(response.trim() == 'true') { alert('Deleted'); location.reload(); }
        else{ alert('Delete failed'); }
    })
    .catch(function(){
        alert('Delete failed');
    });
}
</script>
)";

// New directory handler (only included when can-overwrite/create is allowed).
// Prompts for a directory name via JavaScript, then POSTs it to the current
// directory's /.mkdir endpoint.
static constexpr const char *HTML_JS_MKDIR = R"(
<script>
document.addEventListener('DOMContentLoaded', function(){
    var btn = document.getElementById('mkdirbtn');
    if(!btn) { return; }
    btn.addEventListener('click', function(){
        var name = prompt('New directory name:');
        if(name === null) { return; }
        name = name.trim();
        if(name === '') { alert('Name is empty.'); return; }
        fetch('./.mkdir?e=' + encodeURIComponent(name), {
            method: 'POST',
            headers: { 'X-CSRFToken': getCookie('CSRFToken') }
        })
        .then(function(response){
            if(!response.ok) { throw new Error('HTTP ' + response.status); }
            return response.text();
        })
        .then(function(response){
            if(response.trim() == 'true') { alert('Directory created'); location.reload(); }
            else{ alert('Directory not created'); }
        })
        .catch(function(){
            alert('Directory not created');
        });
    });
});
</script>
)";

