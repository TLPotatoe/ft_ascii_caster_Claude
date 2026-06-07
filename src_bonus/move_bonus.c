#include "ft_ascii_caster_bonus.h"

/* Bonus — vrai si la case (x,y) est un mur ou hors carte. */
static int	is_blocked(t_game *g, double x, double y)
{
	int	ix;
	int	iy;

	if (x < 0 || y < 0)
		return (1);
	ix = (int)x;
	iy = (int)y;
	if (iy >= g->map.height || ix >= (int)ft_strlen(g->map.grid[iy]))
		return (1);
	return (g->map.grid[iy][ix] == '1');
}

/* Bonus — applique un déplacement avec collisions : chaque axe est tenté
   indépendamment, ce qui fait glisser le joueur le long des murs. */
static void	try_move(t_game *g, double dx, double dy)
{
	if (!is_blocked(g, g->pos_x + dx, g->pos_y))
		g->pos_x += dx;
	if (!is_blocked(g, g->pos_x, g->pos_y + dy))
		g->pos_y += dy;
}

/* Avance/recule le long du vecteur direction (signe = +1 avant, -1 arrière). */
void	player_forward(t_game *g, double sign)
{
	try_move(g, g->dir_x * MOVE_SPEED * sign, g->dir_y * MOVE_SPEED * sign);
}

/* Déplacement latéral (strafe) perpendiculaire à la direction. */
void	player_strafe(t_game *g, double sign)
{
	try_move(g, g->plane_x / PLANE_LEN * MOVE_SPEED * sign,
		g->plane_y / PLANE_LEN * MOVE_SPEED * sign);
}
