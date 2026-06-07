#include "ft_ascii_caster.h"

/* Vrai si (x,y) est un mur ou hors carte (sécurise la fin de boucle DDA). */
static int	is_wall(t_game *g, int x, int y)
{
	if (y < 0 || y >= g->map.height || x < 0)
		return (1);
	if (x >= (int)ft_strlen(g->map.grid[y]))
		return (1);
	return (g->map.grid[y][x] == '1');
}

/* Initialise direction du rayon, case de départ et distances delta pour la
   colonne dont le décalage caméra est cam (-1 à gauche, +1 à droite). */
void	ray_setup(t_game *g, t_ray *r, double cam)
{
	r->dir_x = g->dir_x + g->plane_x * cam;
	r->dir_y = g->dir_y + g->plane_y * cam;
	r->map_x = (int)g->pos_x;
	r->map_y = (int)g->pos_y;
	r->delta_x = 1e30;
	r->delta_y = 1e30;
	if (r->dir_x != 0)
		r->delta_x = d_abs(1.0 / r->dir_x);
	if (r->dir_y != 0)
		r->delta_y = d_abs(1.0 / r->dir_y);
}

/* Calcule le pas (step) et la distance au premier bord de cellule (side). */
void	ray_step(t_game *g, t_ray *r)
{
	if (r->dir_x < 0)
	{
		r->step_x = -1;
		r->side_x = (g->pos_x - r->map_x) * r->delta_x;
	}
	else
	{
		r->step_x = 1;
		r->side_x = (r->map_x + 1.0 - g->pos_x) * r->delta_x;
	}
	if (r->dir_y < 0)
	{
		r->step_y = -1;
		r->side_y = (g->pos_y - r->map_y) * r->delta_y;
	}
	else
	{
		r->step_y = 1;
		r->side_y = (r->map_y + 1.0 - g->pos_y) * r->delta_y;
	}
}

/* Boucle DDA : avance de cellule en cellule jusqu'au premier mur, puis calcule
   la distance perpendiculaire (corrige l'effet fish-eye). */
void	ray_cast(t_game *g, t_ray *r)
{
	r->side = 0;
	while (!is_wall(g, r->map_x, r->map_y))
	{
		if (r->side_x < r->side_y)
		{
			r->side_x += r->delta_x;
			r->map_x += r->step_x;
			r->side = 0;
		}
		else
		{
			r->side_y += r->delta_y;
			r->map_y += r->step_y;
			r->side = 1;
		}
	}
	if (r->side == 0)
		r->dist = r->side_x - r->delta_x;
	else
		r->dist = r->side_y - r->delta_y;
	if (r->dist < 0.0001)
		r->dist = 0.0001;
}
