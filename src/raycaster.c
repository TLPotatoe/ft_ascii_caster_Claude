#include "ft_ascii_caster.h"
#include <unistd.h>

/* Valeur absolue (fabs n'est pas dans les fonctions autorisées). */
static double	d_abs(double v)
{
	if (v < 0)
		return (-v);
	return (v);
}

/* Caractère d'ombrage selon la distance : plus c'est proche, plus c'est dense
   (cf. ambiguïté A6 dans claude.md). */
static char	shade_char(double dist)
{
	if (dist < 2.0)
		return ('@');
	if (dist < 4.0)
		return ('#');
	if (dist < 7.0)
		return ('O');
	if (dist < 12.0)
		return ('x');
	return ('.');
}

/* Accès à la grille : 1 si mur ou hors carte (sécurise la sortie de boucle),
   0 si case franchissable. */
static int	is_wall(t_game *g, int x, int y)
{
	if (y < 0 || y >= g->map.height || x < 0)
		return (1);
	if (x >= (int)ft_strlen(g->map.grid[y]))
		return (1);
	return (g->map.grid[y][x] == '1');
}

/* Lance un rayon pour la colonne écran sx via l'algorithme DDA et renseigne
   drawStart, drawEnd et le caractère de mur. */
static void	cast_column(t_game *g, int sx, int *start, int *end, char *ch)
{
	double	cam;
	double	rdx;
	double	rdy;
	double	ddx;
	double	ddy;
	double	sdx;
	double	sdy;
	int		mx;
	int		my;
	int		stepx;
	int		stepy;
	int		side;
	double	dist;
	int		lh;

	cam = 2.0 * sx / (double)SCR_W - 1.0;
	rdx = g->dir_x + g->plane_x * cam;
	rdy = g->dir_y + g->plane_y * cam;
	mx = (int)g->pos_x;
	my = (int)g->pos_y;
	if (rdx == 0)
		ddx = 1e30;
	else
		ddx = d_abs(1.0 / rdx);
	if (rdy == 0)
		ddy = 1e30;
	else
		ddy = d_abs(1.0 / rdy);
	if (rdx < 0)
	{
		stepx = -1;
		sdx = (g->pos_x - mx) * ddx;
	}
	else
	{
		stepx = 1;
		sdx = (mx + 1.0 - g->pos_x) * ddx;
	}
	if (rdy < 0)
	{
		stepy = -1;
		sdy = (g->pos_y - my) * ddy;
	}
	else
	{
		stepy = 1;
		sdy = (my + 1.0 - g->pos_y) * ddy;
	}
	side = 0;
	while (!is_wall(g, mx, my))
	{
		if (sdx < sdy)
		{
			sdx += ddx;
			mx += stepx;
			side = 0;
		}
		else
		{
			sdy += ddy;
			my += stepy;
			side = 1;
		}
	}
	if (side == 0)
		dist = sdx - ddx;
	else
		dist = sdy - ddy;
	if (dist < 0.0001)
		dist = 0.0001;
	lh = (int)(SCR_H / dist);
	*start = -lh / 2 + SCR_H / 2;
	*end = lh / 2 + SCR_H / 2;
	if (*start < 0)
		*start = 0;
	if (*end >= SCR_H)
		*end = SCR_H - 1;
	*ch = shade_char(dist);
}

/* Construit le buffer d'affichage (frame) puis l'écrit en un seul write.
   Espaces au-dessus du mur = plafond, en-dessous = sol. */
void	render_frame(t_game *game)
{
	int		start[SCR_W];
	int		end[SCR_W];
	char	col[SCR_W];
	int		x;
	int		y;
	char	*p;

	x = 0;
	while (x < SCR_W)
	{
		cast_column(game, x, &start[x], &end[x], &col[x]);
		x++;
	}
	p = game->frame;
	*p++ = '\033';
	*p++ = '[';
	*p++ = 'H';
	y = 0;
	while (y < SCR_H)
	{
		x = 0;
		while (x < SCR_W)
		{
			if (y < start[x] || y > end[x])
				*p++ = ' ';
			else
				*p++ = col[x];
			x++;
		}
		*p++ = '\n';
		y++;
	}
	write(1, game->frame, (size_t)(p - game->frame));
}
