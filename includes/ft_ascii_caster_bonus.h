#ifndef FT_ASCII_CASTER_BONUS_H
# define FT_ASCII_CASTER_BONUS_H

# include <termios.h>
# include <stddef.h>

/* Résolution d'affichage fixe (cf. ambiguïté A1 dans claude.md). */
# define SCR_W 80
# define SCR_H 40

/* Champ de vision ~60° -> longueur du plan caméra = tan(30°). */
# define PLANE_LEN 0.5773502691896257

# define MOVE_SPEED 0.10
# define ROT_SPEED 0.06
# define FRAME_US 16000

# define KEY_ESC 27
# define KEY_Q 'q'
# define KEY_CTRL_C 3
# define KEY_CTRL_D 4

/* Bonus mini-carte : marge depuis le coin haut-gauche. */
# define MM_OX 1
# define MM_OY 1

typedef struct s_map
{
	char	**grid;
	int		height;
	int		width;
}	t_map;

typedef struct s_game
{
	t_map			map;
	double			pos_x;
	double			pos_y;
	double			dir_x;
	double			dir_y;
	double			plane_x;
	double			plane_y;
	struct termios	orig_term;
	int				raw_active;
	int				running;
	char			*frame;
}	t_game;

/* parse_map.c */
int		load_map(const char *path, t_game *game);

/* parse_check.c */
int		validate_map(t_game *game);
int		set_player(t_game *game, int x, int y, char c);

/* terminal.c */
int		term_raw_mode(t_game *game);
void	term_restore(t_game *game);

/* raycaster.c */
void	render_frame(t_game *game);

/* minimap.c (bonus) */
void	draw_minimap(t_game *game, char *grid);

/* player.c */
int		handle_input(t_game *game);

/* utils.c */
size_t	ft_strlen(const char *s);
void	ft_putstr_fd(const char *s, int fd);
void	error_exit(t_game *game, const char *msg);
void	free_game(t_game *game);

#endif
