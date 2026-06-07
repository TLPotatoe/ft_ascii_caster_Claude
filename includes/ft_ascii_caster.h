#ifndef FT_ASCII_CASTER_H
# define FT_ASCII_CASTER_H

# include <termios.h>
# include <stddef.h>

/* Résolution d'affichage fixe (cf. ambiguïté A1 dans claude.md).
   On ne lit pas la taille réelle du terminal (ioctl interdit). */
# define SCR_W 80
# define SCR_H 40

/* Champ de vision ~60° -> longueur du plan caméra = tan(30°). */
# define PLANE_LEN 0.5773502691896257

/* Vitesses (unités de grille / radians par frame). */
# define MOVE_SPEED 0.10
# define ROT_SPEED 0.06

/* Délai entre deux frames (microsecondes) -> ~60 FPS. */
# define FRAME_US 16000

/* Codes de touches. */
# define KEY_ESC 27
# define KEY_Q 'q'
# define KEY_CTRL_C 3
# define KEY_CTRL_D 4

/* Grille de la carte. */
typedef struct s_map
{
	char	**grid;		/* lignes de la carte (terminées par '\0') */
	int		height;		/* nombre de lignes */
	int		width;		/* largeur (longueur de la ligne la plus longue) */
}	t_map;

/* État complet du jeu. */
typedef struct s_game
{
	t_map			map;
	double			pos_x;		/* position joueur (x = colonne) */
	double			pos_y;		/* position joueur (y = ligne) */
	double			dir_x;		/* vecteur direction */
	double			dir_y;
	double			plane_x;	/* plan caméra (perpendiculaire à dir) */
	double			plane_y;
	struct termios	orig_term;	/* config terminal d'origine (restauration) */
	int				raw_active;	/* 1 si le mode raw est actif */
	int				running;	/* 0 -> demande de sortie */
	char			*frame;		/* buffer d'affichage réutilisé chaque frame */
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

/* player.c */
int		handle_input(t_game *game);

/* utils.c */
size_t	ft_strlen(const char *s);
void	ft_putstr_fd(const char *s, int fd);
char	*put_uint(char *dst, unsigned int n);
void	error_exit(t_game *game, const char *msg);
void	free_game(t_game *game);

#endif
