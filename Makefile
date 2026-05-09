CC = c++
CFLAGS = -Wall -Wextra -Werror -O3 -std=c++98 -g

NAME = webserv

SRC = CgiHandler.cpp  HttpRequest.cpp  HttpResponse.cpp  LocationConfig.cpp  main.cpp  RequestHandler.cpp  ServerConfig.cpp
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

re: fclean all

.PHONY: all clean fclean re