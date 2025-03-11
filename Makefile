NAME = ft_ping

CC= clang
CFLAGS= -Wall -Wextra -Werror
LIBS= -lm

SRC=	main.c \
		display.c \
		exit.c \
		ft_ping.c \
		network.c \
		packet.c \
		parsing.c \

OBJ= $(SRC:.c=.o)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all
all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(LIBS) -o $(NAME) $(OBJ)

.PHONY: clean
clean:
	rm -rf $(OBJ)

.PHONY: fclean
fclean: clean
	rm -rf $(NAME)

.PHONY: re
re: fclean $(NAME)