#ifndef FT_ASCII_CASTER_H
# define FT_ASCII_CASTER_H

# include <stddef.h>
# include <termios.h>

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

/* Grille de la carte (grid: lignes terminées par '\0' ; width: ligne la plus
   longue). */
typedef struct s_map
{
	char	**grid;
	int		height;
	int		width;
}	t_map;

/* État complet du jeu : position/direction joueur, plan caméra, config terminal
   d'origine, drapeaux de boucle et buffer d'affichage réutilisé. */
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

/* État de travail d'un rayon DDA (une colonne écran). */
typedef struct s_ray
{
	double	dir_x;
	double	dir_y;
	double	delta_x;
	double	delta_y;
	double	side_x;
	double	side_y;
	int		map_x;
	int		map_y;
	int		step_x;
	int		step_y;
	int		side;
	double	dist;
}	t_ray;

/* parse_read.c */
char		*read_all(int fd);

/* parse_map.c */
int			load_map(const char *path, t_game *game);

/* parse_check.c */
int			validate_map(t_game *game);

/* map_closed.c */
int			map_is_closed(t_game *game);

/* terminal.c */
int			term_raw_mode(t_game *game);
void		term_restore(t_game *game);

/* raycaster.c */
void		ray_setup(t_game *g, t_ray *r, double cam);
void		ray_step(t_game *g, t_ray *r);
void		ray_cast(t_game *g, t_ray *r);

/* render.c */
void		render_frame(t_game *game);

/* player.c */
int			handle_input(t_game *game);

/* utils.c */
size_t		ft_strlen(const char *s);
void		ft_putstr_fd(const char *s, int fd);
void		error_exit(t_game *game, const char *msg);
void		free_game(t_game *game);
double		d_abs(double v);

#endif
