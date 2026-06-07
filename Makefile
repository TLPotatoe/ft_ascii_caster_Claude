NAME		= ft_ascii_caster

CC			= cc
CFLAGS		= -Wall -Wextra -Werror
INCLUDES	= -Iincludes

SRC_DIR		= src
SRCS		= main.c parse_map.c parse_check.c terminal.c \
			  raycaster.c player.c utils.c
OBJS		= $(addprefix $(SRC_DIR)/, $(SRCS:.c=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -lm -o $(NAME)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
