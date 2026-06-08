#include "ft_ascii_caster_bonus.h"
#include <math.h>

/* Fait pivoter direction et plan caméra de l'angle donné (radians). */
void	rotate_cam(t_game *g, double a)
{
	double	odx;
	double	opx;

	odx = g->dir_x;
	g->dir_x = g->dir_x * cos(a) - g->dir_y * sin(a);
	g->dir_y = odx * sin(a) + g->dir_y * cos(a);
	opx = g->plane_x;
	g->plane_x = g->plane_x * cos(a) - g->plane_y * sin(a);
	g->plane_y = opx * sin(a) + g->plane_y * cos(a);
}

/* Applique une touche WASD (A/D = strafe, W/S = avant/arrière). */
void	apply_move(t_game *game, char c)
{
	if (c == 'w' || c == 'W')
		player_forward(game, 1.0);
	else if (c == 's' || c == 'S')
		player_forward(game, -1.0);
	else if (c == 'a' || c == 'A')
		player_strafe(game, -1.0);
	else if (c == 'd' || c == 'D')
		player_strafe(game, 1.0);
}

/* Touches de mode de rendu : r = lettres couleur, t = densité couleur,
   y = demi-bloc (double résolution verticale), u = quadrants (double les deux
   axes). */
void	apply_mode(t_game *game, char c)
{
	if (c == KEY_R)
		game->mode = MODE_FACE;
	else if (c == KEY_T)
		game->mode = MODE_SHADE;
	else if (c == KEY_Y)
		game->mode = MODE_HALF;
	else if (c == KEY_U)
		game->mode = MODE_QUAD;
}
