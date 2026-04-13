# Guide détaillé du projet Webserv (École 42)

## 1. Le but du projet

Le projet **webserv** consiste à créer ton propre serveur web en C++
(comme un mini Nginx ou Apache).

Le programme doit : 1. ouvrir un serveur TCP 2. écouter sur un port 3.
accepter des connexions clients 4. recevoir une requête HTTP 5. analyser
la requête 6. envoyer une réponse HTTP correcte

Pipeline général :

    navigateur → requête HTTP → ton serveur → réponse HTTP → navigateur

Exemple requête :

    GET /index.html HTTP/1.1
    Host: localhost

Réponse :

    HTTP/1.1 200 OK
    Content-Type: text/html
    Content-Length: 123

    <html>...</html>

------------------------------------------------------------------------

# 2. Architecture globale

Pipeline du serveur :

    socket()
    ↓
    bind()
    ↓
    listen()
    ↓
    accept()
    ↓
    receive HTTP request
    ↓
    parse request
    ↓
    generate response
    ↓
    send response

Fonctions réseau importantes :

  Fonction   Rôle
  ---------- --------------------
  socket     créer le serveur
  bind       associer un port
  listen     écouter
  accept     accepter un client
  recv       lire la requête
  send       envoyer la réponse

------------------------------------------------------------------------

# 3. Fichier de configuration

Ton serveur doit lire un fichier de configuration style nginx.

Exemple :

    server {
        listen 8080;
        host 127.0.0.1;

        root ./www;
        index index.html;

        location /images {
            root ./images;
        }
    }

Étapes : 1. lire le fichier 2. parser le contenu 3. stocker les
paramètres

Classes possibles :

    ServerConfig
    LocationConfig
    ConfigParser

------------------------------------------------------------------------

# 4. Gestion des requêtes HTTP

Méthodes minimales à supporter :

  Méthode   Rôle
  --------- -------------------
  GET       récupérer fichier
  POST      envoyer données
  DELETE    supprimer fichier

------------------------------------------------------------------------

## Exemple GET

Navigateur :

    GET /index.html HTTP/1.1
    Host: localhost

Serveur : 1. trouve le fichier 2. ouvre le fichier 3. envoie le contenu

------------------------------------------------------------------------

## Exemple POST

    POST /upload HTTP/1.1
    Content-Length: 20

Serveur : 1. lire le body 2. traiter les données

------------------------------------------------------------------------

## Exemple DELETE

    DELETE /file.txt HTTP/1.1

Serveur : supprimer le fichier.

------------------------------------------------------------------------

# 5. Réponses HTTP

Format :

    HTTP/1.1 200 OK
    Content-Type: text/html
    Content-Length: 123

    <body>

Codes importants :

  Code   Signification
  ------ --------------------
  200    OK
  201    created
  204    no content
  400    bad request
  403    forbidden
  404    not found
  405    method not allowed
  500    server error

------------------------------------------------------------------------

# 6. Fichiers statiques

Ton serveur doit servir :

    html
    css
    js
    images

Exemple :

    GET /style.css

Serveur :

    ./www/style.css

Réponse :

    Content-Type: text/css

------------------------------------------------------------------------

# 7. CGI

Support des scripts CGI.

Exemple :

    /cgi-bin/script.py

Le serveur doit : 1. lancer un process 2. exécuter le script 3.
récupérer la sortie

Fonctions utilisées :

    fork()
    execve()
    pipe()
    dup2()

------------------------------------------------------------------------

# 8. Serveur non bloquant

Le serveur doit gérer plusieurs clients simultanément.

Utiliser :

    select()

ou

    poll()

Principe :

    1 serveur
    ↓
    plusieurs clients simultanément

Schéma :

    client 1
    client 2
    client 3
       ↓
    select()
       ↓
    traiter sockets prêtes

------------------------------------------------------------------------

# 9. Gestion des erreurs

Erreurs typiques :

  erreur   comportement
  -------- ------------------
  404      page non trouvée
  403      accès interdit
  500      erreur interne

Configuration possible :

    error_page 404 /404.html;

------------------------------------------------------------------------

# 10. Autoindex

Si dossier demandé :

    GET /images/

et pas d'index :

    file1.jpg
    file2.png

------------------------------------------------------------------------

# 11. Upload

Avec POST :

    upload fichier

Le serveur doit enregistrer le fichier sur le disque.

------------------------------------------------------------------------

# 12. Tests

Tester avec navigateur ou curl :

    curl http://localhost:8080

------------------------------------------------------------------------

# 13. Organisation du code

Classes possibles :

    Server
    Socket
    Client
    HttpRequest
    HttpResponse
    Config
    CGIHandler

Architecture :

    main
     ↓
    config parser
     ↓
    create servers
     ↓
    event loop (select/poll)
     ↓
    handle request
     ↓
    send response

------------------------------------------------------------------------

# 14. Difficulté du projet

Domaines impliqués :

  Domaine      Difficulté
  ------------ --------------
  réseau       sockets
  HTTP         parsing
  OS           fork / exec
  asynchrone   select
  C++          architecture

------------------------------------------------------------------------

# 15. Ce que les correcteurs testent

    GET
    POST
    DELETE
    CGI
    multiple clients
    errors
    config file

------------------------------------------------------------------------

# Ordre efficace pour implémenter le projet

## Étape 1 --- Serveur TCP minimal

Fonctions :

    socket
    setsockopt
    bind
    listen
    accept

Test :

    telnet localhost 8080

------------------------------------------------------------------------

## Étape 2 --- Recevoir les données

Fonction :

    recv()

Afficher la requête reçue.

------------------------------------------------------------------------

## Étape 3 --- Envoyer une réponse simple

Réponse :

    HTTP/1.1 200 OK
    Content-Type: text/plain
    Content-Length: 5

    hello

Fonction :

    send()

------------------------------------------------------------------------

## Étape 4 --- Gérer plusieurs clients

Utiliser :

    select()

ou

    poll()

------------------------------------------------------------------------

## Étape 5 --- Parser HTTP

Extraire :

    method
    path
    headers
    body

Structure :

    class HttpRequest
    {
    method
    path
    headers
    body
    }

------------------------------------------------------------------------

## Étape 6 --- Servir les fichiers

Fonctions :

    open
    read
    stat

------------------------------------------------------------------------

## Étape 7 --- Parser la configuration

Lire :

    listen
    root
    index
    location
    error_page

Structure :

    class ServerConfig
    {
    port
    root
    index
    locations
    }

------------------------------------------------------------------------

## Étape 8 --- Routes (location)

Exemple :

    location /images {
        root ./img;
    }

------------------------------------------------------------------------

## Étape 9 --- Méthodes HTTP

Supporter :

    GET
    POST
    DELETE

DELETE utilise :

    remove()

------------------------------------------------------------------------

## Étape 10 --- CGI

Fonctions :

    fork()
    execve()
    pipe()
    dup2()

Processus :

    client request
    ↓
    serveur fork
    ↓
    exec script
    ↓
    script produit output
    ↓
    serveur renvoie output

------------------------------------------------------------------------

## Étape 11 --- Upload

POST peut contenir :

    multipart/form-data

------------------------------------------------------------------------

## Étape 12 --- Pages d'erreur

    404 Not Found

Configuration possible :

    error_page 404 /404.html

------------------------------------------------------------------------

## Étape 13 --- Autoindex

Si dossier demandé :

    GET /images/

Le serveur peut générer une liste de fichiers.

------------------------------------------------------------------------

# Architecture finale

    main
     ↓
    config parser
     ↓
    server manager
     ↓
    event loop (poll/select)
     ↓
    client manager
     ↓
    request parser
     ↓
    response builder
     ↓
    CGI handler

------------------------------------------------------------------------

# Difficultés principales

1.  parsing HTTP\
2.  select / poll\
3.  CGI\
4.  parsing config

------------------------------------------------------------------------

# Temps moyen pour finir

  niveau         temps
  -------------- ---------------
  bon étudiant   2--3 semaines
  normal         1 mois
  lent           2 mois
