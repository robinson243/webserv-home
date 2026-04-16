👤 Personne 1 — Config Parser

Fichiers : LocationConfig.cpp, ServerConfig.cpp, ConfigParser.cpp

Tâches dans l'ordre :

    Constructeurs + getters/setters de LocationConfig et ServerConfig

    Lire et tokenizer le fichier .conf

    Parser les blocs server { } → remplir un ServerConfig

    Parser les blocs location { } imbriqués → remplir les LocationConfig

    Gérer les erreurs de config (port invalide, directive inconnue, accolades non fermées)

    Écrire un fichier .conf de test complet

Livrable : une fonction parse(fichier.conf) qui retourne un std::vector<ServerConfig> prêt à l'emploi

-----------------------------------

👤 Personne 2 — Réseau & Event Loop

Fichiers : Server.cpp, EventLoop.cpp (ou WebServer.cpp)

Tâches dans l'ordre :

    Créer et configurer les sockets TCP (socket, setsockopt, bind, listen, fcntl O_NONBLOCK)

    Gérer plusieurs sockets serveurs simultanément (un par port dans la config)

    Implémenter la boucle événementielle avec poll() ou select()

    Distinguer fd serveur (→ accept()) vs fd client (→ recv()/send())

    Gérer les connexions clients : buffer de réception, détection de fin de requête

    Dispatcher vers le HTTP Handler quand la requête est complète

Livrable : une boucle qui accepte des connexions, reçoit des données brutes et les passe à la Personne 3

----------------------------------

👤 Personne 3 — HTTP Handler & CGI

Fichiers : HttpRequest.cpp, HttpResponse.cpp, RequestHandler.cpp, CgiHandler.cpp

Tâches dans l'ordre :

    Constructeurs + getters/setters de HttpRequest et HttpResponse

    Parser une requête HTTP brute → remplir HttpRequest (request line, headers, body)

    Implémenter GET : servir un fichier statique, gérer autoindex

    Implémenter POST : gérer upload, respecter client_max_body_size

    Implémenter DELETE : supprimer un fichier

    Construire les HttpResponse avec les bons codes et headers

    CGI : fork() + execve() + pipe() + timeout

Livrable : une fonction handleRequest(HttpRequest, ServerConfig) → retourne une HttpResponse sérialisée