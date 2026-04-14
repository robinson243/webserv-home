# Webserv --- Plan clair et précis (École 42)

Ce document explique **le plan clair pour implémenter le projet
Webserv** étape par étape.

------------------------------------------------------------------------

# Vue globale du projet

Le projet peut être découpé en 6 grands blocs :

    1. Serveur TCP
    2. Event loop (poll/select)
    3. Réception et parsing HTTP
    4. Génération de réponse HTTP
    5. Configuration du serveur
    6. CGI

------------------------------------------------------------------------

# 1 --- Créer le serveur TCP

Objectif : accepter une connexion.

Fonctions utilisées :

    socket()
    setsockopt()
    bind()
    listen()
    accept()

Créer une classe :

    ServerSocket

Logique :

    socket → bind → listen

Test :

    telnet localhost 8080

------------------------------------------------------------------------

# 2 --- Gérer plusieurs clients

Utiliser :

    poll()

Boucle principale :

    while (true)
    {
        poll(sockets)
    }

Structure :

    vector<pollfd>

Quand poll indique un socket prêt :

    si server socket → accept()
    si client socket → recv()

------------------------------------------------------------------------

# 3 --- Recevoir une requête HTTP

Exemple :

    GET /index.html HTTP/1.1
    Host: localhost
    Connection: keep-alive

Lire avec :

    recv()

Parser :

    method
    path
    version
    headers
    body

Classe :

    HttpRequest

------------------------------------------------------------------------

# 4 --- Générer une réponse HTTP

Classe :

    HttpResponse

Structure :

    status code
    headers
    body

Exemple :

    HTTP/1.1 200 OK
    Content-Type: text/html
    Content-Length: 20

    <html>hello</html>

------------------------------------------------------------------------

# 5 --- Servir des fichiers

Requête :

    GET /index.html

Étapes :

1.  trouver fichier
2.  open()
3.  read()
4.  send()

Fonctions :

    open()
    read()
    stat()
    send()

------------------------------------------------------------------------

# 6 --- Parser la configuration

Exemple :

    server {
        listen 8080;
        root ./www;
        index index.html;
    }

Parser :

    port
    root
    index
    location
    error_page

Classes :

    ConfigParser
    ServerConfig
    LocationConfig

------------------------------------------------------------------------

# 7 --- Gérer les routes

Exemple :

    location /images {
        root ./img;
    }

Requête :

    GET /images/cat.png

Chemin réel :

    ./img/cat.png

------------------------------------------------------------------------

# 8 --- Méthodes HTTP

    GET
    POST
    DELETE

DELETE :

    remove()

------------------------------------------------------------------------

# 9 --- Gestion des erreurs

    404 Not Found
    403 Forbidden
    405 Method Not Allowed
    500 Internal Server Error

------------------------------------------------------------------------

# 10 --- CGI

Exemple :

    /cgi-bin/script.py

Fonctions :

    fork()
    execve()
    pipe()
    dup2()

Flux :

    client request
    ↓
    server fork
    ↓
    exec script
    ↓
    script output
    ↓
    send response

------------------------------------------------------------------------

# Architecture finale

    main
     ↓
    config parser
     ↓
    create servers
     ↓
    event loop (poll)
     ↓
    accept clients
     ↓
    receive request
     ↓
    parse request
     ↓
    build response
     ↓
    send response

------------------------------------------------------------------------

# Classes recommandées

    Server
    Socket
    Client
    HttpRequest
    HttpResponse
    ConfigParser
    ServerConfig
    LocationConfig
    CGIHandler

------------------------------------------------------------------------

# Ordre exact d'implémentation

    1. socket / bind / listen
    2. accept client
    3. recv request
    4. send simple HTTP response
    5. poll loop
    6. HTTP parser
    7. static files
    8. config parser
    9. routes (location)
    10. POST / DELETE
    11. CGI

------------------------------------------------------------------------

# Parties les plus difficiles

    1. Parser HTTP correctement
    2. Gérer poll()
    3. Implémenter CGI
    4. Parser la configuration
