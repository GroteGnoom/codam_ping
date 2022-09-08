NAME = ping
SRC = main.c
INC = ping.h

ifdef DEBUG
	CC=clang
	ASAN_OPTIONS='detect_leaks=1'
	FLAGS = -Wall -Wextra -fsanitize=address -fsanitize=undefined -DDEBUG=1 -g -std=c++98 -pedantic
else
	CC=gcc
	FLAGS = -Wall -Wextra -Werror
endif

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.c $(INC)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

test: all
	tshark -f "icmp" -a duration:2 &
	sleep 1;./ping
	sleep 2

re: fclean all
.PHONY: all clean fclean 
