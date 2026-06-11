NAME		= ft_ascii_caster

CC			= cc
CFLAGS		= -g -Wall -Wextra -Werror
INCLUDES	= -Iincludes

SRC_DIR		= src
SRCS		= main.c parse_read.c parse_map.c parse_check.c map_closed.c \
			  terminal.c raycaster.c render.c player.c utils.c
OBJS		= $(addprefix $(SRC_DIR)/, $(SRCS:.c=.o))

BONUS_NAME	= ft_ascii_caster_bonus
BSRC_DIR	= src_bonus
BSRCS		= main_bonus.c parse_read_bonus.c parse_map_bonus.c \
			  parse_check_bonus.c map_closed_bonus.c terminal_bonus.c \
			  termsize_bonus.c resize_bonus.c raycaster_bonus.c \
			  render_bonus.c render_half_bonus.c halfflush_bonus.c \
			  render_quad_bonus.c quadflush_bonus.c \
			  glyph_bonus.c color_bonus.c palette_bonus.c minimap_bonus.c \
			  move_bonus.c player_bonus.c input_bonus.c utils_bonus.c
BOBJS		= $(addprefix $(BSRC_DIR)/, $(BSRCS:.c=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -lm -o $(NAME)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

bonus: $(BONUS_NAME)

$(BONUS_NAME): $(BOBJS)
	$(CC) $(CFLAGS) $(BOBJS) -lm -o $(BONUS_NAME)

$(BSRC_DIR)/%.o: $(BSRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(BOBJS)

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all

.PHONY: all bonus clean fclean re
