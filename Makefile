CC = c++
CFLAGS = -Wall -Wextra -Werror -O3 -std=c++98 -g

NAME = webserv

SRC = CgiHandler.cpp  EventLoop.cpp  HttpRequest.cpp  HttpResponse.cpp  LocationConfig.cpp  main.cpp  RequestHandler.cpp  ServerConfig.cpp  Server.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
	rm -rf tests_www

re: fclean all

eval :
	bash script.sh

.PHONY: all clean fclean re