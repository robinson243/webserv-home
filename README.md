*This project has been created as part of the 42 curriculum by romukena, oamairi, ydembele.*


## Description

WebServ is an HTTP/1.1 server written in C++98, inspired by Nginx. The goal is to handle HTTP requests from one or multiple virtual servers defined in a `.conf` configuration file.

The server parses the configuration file to build `ServerConfig` and `LocationConfig` objects, which hold all the settings for each server block and its locations. These are then passed to an `EventLoop` that creates the sockets, multiplexes I/O with `poll()`, accepts new clients, reads incoming data, and dispatches complete HTTP requests through `HttpRequest` (parsing), `RequestHandler` (routing and logic for GET, POST, DELETE), and `HttpResponse` (response serialization). CGI scripts are handled separately by `CgiHandler`.

## Architecture

```
.conf file
    │
    ▼
ServerConfig + LocationConfig   (parsing)
    │
    ▼
EventLoop                       (poll(), accept(), recv(), send())
    │
    ├── HttpRequest              (parse raw HTTP)
    │       │
    │       ▼
    │   RequestHandler          (GET / POST / DELETE)
    │       │
    │       ▼
    │   HttpResponse            (serialize and send)
    │
    └── CgiHandler              (fork + exec CGI scripts)
```

## File overview

| File | Role |
|---|---|
| `main.cpp` | Entry point, loads config, starts EventLoop |
| `ServerConfig.cpp/hpp` | Parses and stores `Serveronfig {}` block settings |
| `LocationConfig.cpp/hpp` | Parses and stores `locationonfig {}` block settings |
| `Server.cpp/hpp` | Creates socket, bind, listen, non-blocking mode |
| `EventLoop.cpp/hpp` | Main loop: poll, client management, dispatching |
| `HttpRequest.cpp/hpp` | Parses raw HTTP request |
| `RequestHandler.cpp/hpp` | request and main function of request  |
| `HttpResponse.cpp/hpp` | Builds and serializes HTTP response |
| `CgiHandler.cpp/hpp` | Executes CGI scripts |

## Configuration example

```nginx
server {
    listen       8080;
    server_name  localhost;
    root         ./www;
    index        index.html;
    client_max_body_size 1000;

    error_page 404 /errors/404.html;

    location / {
        methods GET POST;
        autoindex on;
    }

    location /cgi-bin {
        methods GET POST;
        cgi_extension .py /usr/bin/python3;
    }
}
```

## Instructions

```bash
make              
make re           
make clean        
make fclean    

./webserv [config_file]   # run with a config file
./webserv exemple.conf    # example
```

## Testing

```bash
# GET request
curl http://localhost:8080/

# POST request
curl -X POST http://localhost:8080/upload -d "data=hello"

# DELETE request
curl -X DELETE http://localhost:8080/upload/file.txt

# Stress test
siege -b -c 10 -r 100 http://localhost:8080/

# Check for fd leaks
valgrind --track-fds=yes ./webserv exemple.conf
```

## Resources



**AI usage:** We used AI to help distribute the workload evenly between team members, to generate `.conf` files for testing various configurations, and to write test cases (curl commands, edge cases, stress scenarios).
