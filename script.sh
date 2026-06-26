
mkdir -p tests_www/{static,forms,upload,autoindex,errors,cgi-bin} && \
cat > tests_www/static/index.html <<'EOF'
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>webserv static test</title>
  <link rel="stylesheet" href="/static/style.css">
</head>
<body>
  <h1>Test page webserv</h1>
  <p>Si tu vois cette page, le service de fichiers statiques fonctionne.</p>
  <a href="/static/hello.txt">Télécharger hello.txt</a>
  <script src="/static/app.js"></script>
</body>
</html>
EOF
cat > tests_www/static/style.css <<'EOF'
body {
  font-family: Arial, sans-serif;
  background: #f4f4f4;
  color: #222;
  padding: 40px;
}
h1 {
  color: #0a7;
}
a {
  display: inline-block;
  margin-top: 20px;
}
EOF
cat > tests_www/static/app.js <<'EOF'
document.addEventListener('DOMContentLoaded', function () {
  console.log('JS chargé depuis webserv');
});
EOF
cat > tests_www/static/hello.txt <<'EOF'
Bonjour depuis hello.txt
EOF
cat > tests_www/forms/post_form.html <<'EOF'
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>POST test</title>
</head>
<body>
  <h1>Formulaire POST</h1>
  <form action="/submit" method="post">
    <label>Nom: <input type="text" name="name"></label><br><br>
    <label>Message: <textarea name="message"></textarea></label><br><br>
    <button type="submit">Envoyer</button>
  </form>
</body>
</html>
EOF
cat > tests_www/forms/delete_me.txt <<'EOF'
Ce fichier est là pour tester DELETE.
EOF
cat > tests_www/upload/index.html <<'EOF'
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>Upload test</title>
</head>
<body>
  <h1>Upload test</h1>
  <form action="/upload" method="post" enctype="multipart/form-data">
    <input type="file" name="file">
    <button type="submit">Uploader</button>
  </form>
</body>
</html>
EOF
cat > tests_www/autoindex/a.txt <<'EOF'
fichier A
EOF
cat > tests_www/autoindex/b.txt <<'EOF'
fichier B
EOF
cat > tests_www/errors/404.html <<'EOF'
<!DOCTYPE html>
<html lang="fr">
<head><meta charset="UTF-8"><title>404</title></head>
<body><h1>404 Not Found</h1><p>La ressource demandée est introuvable.</p></body>
</html>
EOF
cat > tests_www/errors/500.html <<'EOF'
<!DOCTYPE html>
<html lang="fr">
<head><meta charset="UTF-8"><title>500</title></head>
<body><h1>500 Internal Server Error</h1><p>Erreur interne du serveur.</p></body>
</html>
EOF
cat > tests_www/cgi-bin/env.py <<'EOF'
#!/usr/bin/env python3
import os
print("Content-Type: text/html\r\n")
print("\r\n")
print("<html><body><h1>CGI ENV</h1>")
for key, value in sorted(os.environ.items()):
    print(f"<p><b>{key}</b> = {value}</p>")
print("</body></html>")
EOF
cat > tests_www/cgi-bin/echo.py <<'EOF'
#!/usr/bin/env python3
import sys
body = sys.stdin.read()
print("Content-Type: text/plain\r\n")
print("\r\n")
print("CGI echo OK")
print(body)
EOF
chmod +x tests_www/cgi-bin/env.py tests_www/cgi-bin/echo.py