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

/* Déduit les lignes de début/fin du mur depuis la distance (perspective).
   sh = hauteur écran courante (résolution adaptée au terminal). */
static void	column_height(t_ray *r, int sh, int *start, int *end)
{
	int	lh;

	lh = (int)(sh / r->dist);
	*start = -lh / 2 + sh / 2;
	*end = lh / 2 + sh / 2;
	if (*start < 0)
		*start = 0;
	if (*end >= sh)
		*end = sh - 1;
}

/* Lance le rayon de la colonne sx et remplit la colonne de l'écran avec le
   caractère de face du mur + son palier de couleur selon la distance ; le
   plafond/sol reçoit un espace et la sentinelle BAND_SPACE. */
static void	fill_column(t_game *g, t_screen *scr, int sx)
{
	t_ray	r;
	int		start;
	int		end;
	int		y;

	ray_setup(g, &r, 2.0 * sx / (double)g->scr_w - 1.0);
	ray_step(g, &r);
	ray_cast(g, &r);
	column_height(&r, g->scr_h, &start, &end);
	y = 0;
	while (y < g->scr_h)
	{
		if (y < start || y > end)
		{
			scr->ch[y * g->scr_w + sx] = ' ';
			scr->band[y * g->scr_w + sx] = BAND_SPACE;
		}
		else
		{
			scr->ch[y * g->scr_w + sx] = face_char(&r);
			scr->band[y * g->scr_w + sx] = dist_band(r.dist);
		}
		y++;
	}
}

/* Sérialise la grille écran (curseur en haut, couleur par teinte/nuance, reset
   + saut de ligne par rangée) et l'écrit en un seul write. */
static void	flush_screen(t_game *game, t_screen *scr)
{
	char		*p;
	const char	*prev;
	int			x;
	int			y;

	p = append_str(game->frame, "\033[H");
	y = 0;
	while (y < game->scr_h)
	{
		x = 0;
		prev = 0;
		while (x < game->scr_w)
			p = emit_cell(p, scr, y * game->scr_w + x++, &prev);
		p = append_str(p, "\033[0m");
		*p++ = '\n';
		y++;
	}
	write(1, game->frame, (size_t)(p - game->frame));
}

void	render_frame(t_game *game)
{
	t_screen	scr;
	int			x;

	scr.ch = game->screen;
	scr.band = game->band;
	scr.w = game->scr_w;
	scr.h = game->scr_h;
	x = 0;
	while (x < game->scr_w)
	{
		fill_column(game, &scr, x);
		x++;
	}
	draw_minimap(game, &scr);
	flush_screen(game, &scr);
}
