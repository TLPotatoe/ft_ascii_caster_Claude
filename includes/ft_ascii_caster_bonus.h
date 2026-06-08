#ifndef FT_ASCII_CASTER_BONUS_H
# define FT_ASCII_CASTER_BONUS_H

# include <stddef.h>
# include <termios.h>

/* Bonus — résolution adaptée au terminal (détectée à l'exécution, cf. A1).
   Valeurs de repli si la détection échoue (non-tty) + bornes de sécurité
   pour clamper la taille (et borner l'allocation du buffer). */
# define DEF_W 80
# define DEF_H 40
# define MIN_W 20
# define MIN_H 10
# define MAX_W 800
# define MAX_H 400

/* Bonus couleur : sentinelle plafond/sol et taille max d'un escape ANSI
   (\033[38;5;NNNm). Le buffer de frame est dimensionné dans main_bonus.c
   (pire cas : un escape par cellule + reset/saut de ligne par rangée). */
# define BAND_SPACE 9
# define COLOR_LEN 11

/* Champ de vision ~60° -> longueur du plan caméra = tan(30°). */
# define PLANE_LEN 0.5773502691896257

# define MOVE_SPEED 0.10
# define ROT_SPEED 0.06
# define FRAME_US 16000

/* Bonus resize : re-mesure de la taille tous les RESIZE_POLL frames
   (~0,5 s à 16 ms/frame) — pas de SIGWINCH (signal() interdit). */
# define RESIZE_POLL 30

# define KEY_ESC 27
# define KEY_Q 'q'
# define KEY_CTRL_C 3
# define KEY_CTRL_D 4

/* Bonus mini-carte : marge depuis le coin haut-gauche. */
# define MM_OX 1
# define MM_OY 1

typedef struct s_map
{
	char			**grid;
	int				height;
	int				width;
}					t_map;

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
	int				scr_w;
	int				scr_h;
	char			*frame;
	char			*screen;
	unsigned char	*band;
}					t_game;

typedef struct s_screen
{
	char			*ch;
	unsigned char	*band;
	int				w;
	int				h;
}					t_screen;

typedef struct s_ray
{
	double			dir_x;
	double			dir_y;
	double			delta_x;
	double			delta_y;
	double			side_x;
	double			side_y;
	int				map_x;
	int				map_y;
	int				step_x;
	int				step_y;
	int				side;
	double			dist;
}					t_ray;

/* parse_read_bonus.c */
char				*read_all(int fd);

/* parse_map_bonus.c */
int					load_map(const char *path, t_game *game);

/* parse_check_bonus.c */
int					validate_map(t_game *game);

/* map_closed_bonus.c */
int					map_is_closed(t_game *game);

/* terminal_bonus.c */
int					term_raw_mode(t_game *game);
void				term_restore(t_game *game);

/* termsize_bonus.c */
int					measure_size(int *cols, int *rows);
void				detect_screen_size(t_game *game);

/* resize_bonus.c */
void				apply_size(t_game *game, int cols, int rows);
void				realloc_buffers(t_game *game);
void				handle_resize(t_game *game);

/* raycaster_bonus.c */
void				ray_setup(t_game *g, t_ray *r, double cam);
void				ray_step(t_game *g, t_ray *r);
void				ray_cast(t_game *g, t_ray *r);

/* render_bonus.c */
void				render_frame(t_game *game);

/* color_bonus.c */
int					dist_band(double dist);
const char			*cell_color(char ch, int band);
char				*append_str(char *p, const char *s);
char				*emit_cell(char *p, t_screen *s, int idx, const char **pv);

/* palette_bonus.c */
const char			*face_color(int f, int b);

/* minimap_bonus.c */
void				draw_minimap(t_game *game, t_screen *scr);

/* move_bonus.c */
void				player_forward(t_game *g, double sign);
void				player_strafe(t_game *g, double sign);

/* player_bonus.c */
int					handle_input(t_game *game);

/* utils_bonus.c */
size_t				ft_strlen(const char *s);
void				ft_putstr_fd(const char *s, int fd);
void				error_exit(t_game *game, const char *msg);
void				free_game(t_game *game);
double				d_abs(double v);

#endif
