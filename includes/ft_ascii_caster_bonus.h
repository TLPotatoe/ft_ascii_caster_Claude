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
# define MAX_W 1500
# define MAX_H 250

/* Bonus couleur — code couleur stocké par cellule : 0..23 = mur (face*6 +
   palier de distance), COL_BG = plafond/sol (fond), COL_WHITE = mini-carte.
   CELL_MAX borne le pire cas d'octets par cellule (mode demi-bloc : avant-plan
   + arrière-plan + glyphe 3 octets) pour dimensionner frame. */
# define COL_BG 30
# define COL_WHITE 31
# define CELL_MAX 26

/* Modes de rendu (touches r/t/y/u). */
# define MODE_FACE 0
# define MODE_SHADE 1
# define MODE_HALF 2
# define MODE_QUAD 3

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
# define KEY_R 'r'
# define KEY_T 't'
# define KEY_Y 'y'
# define KEY_U 'u'

/* Accumulateur d'entrée : les octets lus sont parsés par tokens complets (1
   octet, ou 3 pour une flèche ESC [ C/D). Les octets d'une séquence coupée en
   fin de lecture sont conservés (inlen) et complétés à la frame suivante, ce
   qui évite la désynchronisation du flux (queue de flèche lue comme un strafe)
   quand plusieurs séquences s'accumulent (lag + touche maintenue). */
# define INBUF 64

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
	int				mode;
	char			*frame;
	char			*screen;
	unsigned char	*band;
	unsigned char	*band2;
	char			inbuf[INBUF];
	int				inlen;
}					t_game;

typedef struct s_screen
{
	char			*ch;
	unsigned char	*band;
	unsigned char	*band2;
	int				w;
	int				h;
}					t_screen;

typedef struct s_prev
{
	int				fg;
	int				bg;
}					t_prev;

/* Mode quadrants (u) — résultat d'une sous-colonne de rayon : intervalle de
   mur [start, end] en pixels (2*scr_h) + code couleur (face*6 + palier). */
typedef struct s_qcol
{
	int				start;
	int				end;
	int				code;
}					t_qcol;

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
int					handle_resize(t_game *game);

/* raycaster_bonus.c */
void				ray_setup(t_game *g, t_ray *r, double cam);
void				ray_step(t_game *g, t_ray *r);
void				ray_cast(t_game *g, t_ray *r);

/* render_bonus.c */
int					ray_face(t_ray *r);
void				column_height(t_ray *r, int sh, int *start, int *end);
void				render_frame(t_game *game);

/* glyph_bonus.c */
char				wall_glyph(t_game *g, int f, double dist);

/* render_half_bonus.c */
void				render_half(t_game *game);

/* halfflush_bonus.c */
void				flush_half(t_game *game, t_screen *scr);

/* render_quad_bonus.c */
void				render_quad(t_game *game);

/* quadflush_bonus.c */
void				quad_flush(t_game *game, t_screen *scr);

/* color_bonus.c */
int					dist_band(double dist);
const char			*cell_color(int code);
const char			*code_fg(int code);
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
void				rotate_cam(t_game *g, double a);
void				apply_move(t_game *g, char c);
void				apply_mode(t_game *g, char c);

/* input_bonus.c */
int					handle_input(t_game *game);

/* utils_bonus.c */
size_t				ft_strlen(const char *s);
void				ft_putstr_fd(const char *s, int fd);
void				error_exit(t_game *game, const char *msg);
void				free_game(t_game *game);
double				d_abs(double v);

#endif
