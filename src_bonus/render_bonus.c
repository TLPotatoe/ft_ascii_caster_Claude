#include "ft_ascii_caster_bonus.h"
#include <unistd.h>

/* Bonus — caractère selon la FACE du mur touchée par le rayon. Repère y vers le
   bas : +y = Sud. side 0 = mur vertical (E/W), side 1 = horizontal (N/S). */
static char	face_char(t_ray *r)
{
	if (r->side == 0)
	{
		if (r->step_x == 1)
			return ('W');
		return ('E');
	}
	if (r->step_y == 1)
		return ('N');
	return ('S');
}

/* Déduit les lignes de début/fin du mur depuis la distance (perspective). */
static void	column_height(t_ray *r, int *start, int *end)
{
	int	lh;

	lh = (int)(SCR_H / r->dist);
	*start = -lh / 2 + SCR_H / 2;
	*end = lh / 2 + SCR_H / 2;
	if (*start < 0)
		*start = 0;
	if (*end >= SCR_H)
		*end = SCR_H - 1;
}

/* Lance le rayon de la colonne sx et remplit la colonne de l'écran avec le
   caractère de face du mur (plafond/sol en espaces). */
static void	fill_column(t_game *g, char *screen, int sx)
{
	t_ray	r;
	int		start;
	int		end;
	int		y;

	ray_setup(g, &r, 2.0 * sx / (double)SCR_W - 1.0);
	ray_step(g, &r);
	ray_cast(g, &r);
	column_height(&r, &start, &end);
	y = 0;
	while (y < SCR_H)
	{
		if (y < start || y > end)
			screen[y * SCR_W + sx] = ' ';
		else
			screen[y * SCR_W + sx] = face_char(&r);
		y++;
	}
}

/* Sérialise la grille écran (curseur en haut + sauts de ligne) et l'écrit. */
static void	flush_screen(t_game *game, char *screen)
{
	char	*p;
	int		x;
	int		y;

	p = game->frame;
	*p++ = '\033';
	*p++ = '[';
	*p++ = 'H';
	y = 0;
	while (y < SCR_H)
	{
		x = 0;
		while (x < SCR_W)
			*p++ = screen[y * SCR_W + x++];
		*p++ = '\n';
		y++;
	}
	write(1, game->frame, (size_t)(p - game->frame));
}

void	render_frame(t_game *game)
{
	char	screen[SCR_W * SCR_H];
	int		x;

	x = 0;
	while (x < SCR_W)
	{
		fill_column(game, screen, x);
		x++;
	}
	draw_minimap(game, screen);
	flush_screen(game, screen);
}
