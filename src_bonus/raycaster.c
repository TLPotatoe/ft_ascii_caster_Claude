#include "ft_ascii_caster_bonus.h"
#include <unistd.h>

static double	d_abs(double v)
{
	if (v < 0)
		return (-v);
	return (v);
}

/* Bonus — caractère selon la FACE du mur touchée par le rayon.
   Repère y vers le bas : +y = Sud. side 0 = mur vertical (E/W),
   side 1 = mur horizontal (N/S). */
static char	face_char(int side, int stepx, int stepy)
{
	if (side == 0)
	{
		if (stepx == 1)
			return ('W');
		return ('E');
	}
	if (stepy == 1)
		return ('N');
	return ('S');
}

static int	is_wall(t_game *g, int x, int y)
{
	if (y < 0 || y >= g->map.height || x < 0)
		return (1);
	if (x >= (int)ft_strlen(g->map.grid[y]))
		return (1);
	return (g->map.grid[y][x] == '1');
}

/* DDA pour la colonne sx : remplit start, end et le caractère de face. */
static void	cast_column(t_game *g, int sx, int *start, int *end, char *ch)
{
	double	cam;
	double	ray[2];
	double	dd[2];
	double	sd[2];
	int		m[2];
	int		step[2];
	int		side;
	double	dist;
	int		lh;

	cam = 2.0 * sx / (double)SCR_W - 1.0;
	ray[0] = g->dir_x + g->plane_x * cam;
	ray[1] = g->dir_y + g->plane_y * cam;
	m[0] = (int)g->pos_x;
	m[1] = (int)g->pos_y;
	dd[0] = 1e30;
	dd[1] = 1e30;
	if (ray[0] != 0)
		dd[0] = d_abs(1.0 / ray[0]);
	if (ray[1] != 0)
		dd[1] = d_abs(1.0 / ray[1]);
	if (ray[0] < 0)
	{
		step[0] = -1;
		sd[0] = (g->pos_x - m[0]) * dd[0];
	}
	else
	{
		step[0] = 1;
		sd[0] = (m[0] + 1.0 - g->pos_x) * dd[0];
	}
	if (ray[1] < 0)
	{
		step[1] = -1;
		sd[1] = (g->pos_y - m[1]) * dd[1];
	}
	else
	{
		step[1] = 1;
		sd[1] = (m[1] + 1.0 - g->pos_y) * dd[1];
	}
	side = 0;
	while (!is_wall(g, m[0], m[1]))
	{
		if (sd[0] < sd[1])
		{
			sd[0] += dd[0];
			m[0] += step[0];
			side = 0;
		}
		else
		{
			sd[1] += dd[1];
			m[1] += step[1];
			side = 1;
		}
	}
	if (side == 0)
		dist = sd[0] - dd[0];
	else
		dist = sd[1] - dd[1];
	if (dist < 0.0001)
		dist = 0.0001;
	lh = (int)(SCR_H / dist);
	*start = -lh / 2 + SCR_H / 2;
	*end = lh / 2 + SCR_H / 2;
	if (*start < 0)
		*start = 0;
	if (*end >= SCR_H)
		*end = SCR_H - 1;
	*ch = face_char(side, step[0], step[1]);
}

/* Remplit la grille écran (murs/faces + plafond/sol). */
static void	fill_screen(t_game *game, char *screen)
{
	int		start[SCR_W];
	int		end[SCR_W];
	char	col[SCR_W];
	int		x;
	int		y;

	x = -1;
	while (++x < SCR_W)
		cast_column(game, x, &start[x], &end[x], &col[x]);
	y = -1;
	while (++y < SCR_H)
	{
		x = -1;
		while (++x < SCR_W)
		{
			if (y < start[x] || y > end[x])
				screen[y * SCR_W + x] = ' ';
			else
				screen[y * SCR_W + x] = col[x];
		}
	}
}

/* Sérialise la grille écran (avec home
	+ sauts de ligne) puis écrit la frame. */
void	render_frame(t_game *game)
{
	char	screen[SCR_W * SCR_H];
	char	*p;
	int		x;
	int		y;

	fill_screen(game, screen);
	draw_minimap(game, screen);
	p = game->frame;
	*p++ = '\033';
	*p++ = '[';
	*p++ = 'H';
	y = -1;
	while (++y < SCR_H)
	{
		x = -1;
		while (++x < SCR_W)
			*p++ = screen[y * SCR_W + x];
		*p++ = '\n';
	}
	write(1, game->frame, (size_t)(p - game->frame));
}
