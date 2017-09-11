function submit(e, t) {
    event.preventDefault();
    var form = t.parentNode;
    var formData = new FormData();
    for (var i = 0; i < form.children.length; i++) {
        var input = form.children[i];
        if (input.name != undefined && input.name != '') {
            formData.append(input.name, input.value);
        }
    }
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() {
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
            console.info(xmlHttp.responseText);
            window.location.reload();
        }
    }
    xmlHttp.open(form.method, form.action); 
    xmlHttp.send(formData); 
    return false;
}