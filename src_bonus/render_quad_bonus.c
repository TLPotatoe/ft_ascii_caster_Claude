#include "ft_ascii_caster_bonus.h"

/* Lance le rayon de la sous-colonne subx (parmi subw = 2*scr_w) et remplit le
   t_qcol : intervalle de mur en pixels (échelle 2*scr_h) + code couleur. */
static void	cast_sub(t_game *g, t_qcol *c, int subx, int subw)
{
	t_ray	r;

	ray_setup(g, &r, 2.0 * subx / (double)subw - 1.0);
	ray_step(g, &r);
	ray_cast(g, &r);
	c->code = ray_face(&r) * 6 + dist_band(r.dist);
	column_height(&r, 2 * g->scr_h, &c->start, &c->end);
}

/* Masque 2x2 de la cellule cy : bit TL=8, TR=4, BL=2, BR=1. Un sous-pixel est
   allumé si sa ligne (top=2*cy, bot=2*cy+1) tombe dans l'intervalle de mur de
   la sous-colonne gauche (l) ou droite (r). */
static int	cell_mask(t_qcol *l, t_qcol *r, int cy)
{
	int	t;
	int	b;
	int	m;

	t = 2 * cy;
	b = 2 * cy + 1;
	m = (t >= l->start && t <= l->end) << 3;
	m |= (t >= r->start && t <= r->end) << 2;
	m |= (b >= l->start && b <= l->end) << 1;
	m |= (b >= r->start && b <= r->end);
	return (m);
}

/* Écrit une cellule : glyphe = masque (0..15, traduit au flush), couleur =
   teinte de la sous-colonne gauche si elle porte du mur, sinon droite, sinon
   fond (plafond/sol). 0x0A = bits gauche (TL|BL). */
static void	put_cell(t_game *g, t_qcol *l, t_qcol *r, int idx)
{
	int	m;

	m = cell_mask(l, r, idx / g->scr_w);
	g->screen[idx] = (char)m;
	if (m == 0)
		g->band[idx] = COL_BG;
	else if (m & 0x0A)
		g->band[idx] = (unsigned char)l->code;
	else
		g->band[idx] = (unsigned char)r->code;
}

/* Remplit une colonne de cellules : 2 sous-colonnes de rayons (gauche/droite)
   échantillonnent l'horizontale au double de la résolution. */
static void	fill_quad_col(t_game *g, int cx)
{
	t_qcol	l;
	t_qcol	r;
	int		y;

	cast_sub(g, &l, 2 * cx, 2 * g->scr_w);
	cast_sub(g, &r, 2 * cx + 1, 2 * g->scr_w);
	y = 0;
	while (y < g->scr_h)
	{
		put_cell(g, &l, &r, y * g->scr_w + cx);
		y++;
	}
}

/* Mode quadrants (u) : 2x2 sous-pixels par cellule -> double résolution
   horizontale ET verticale, glyphe choisi par couverture (blocs de quadrant).
   La négative space du glyphe = fond du terminal, qui sert de plafond/sol. */
void	render_quad(t_game *game)
{
	t_screen	scr;
	int			x;

	scr.ch = game->screen;
	scr.band = game->band;
	scr.band2 = game->band2;
	scr.w = game->scr_w;
	scr.h = game->scr_h;
	x = 0;
	while (x < game->scr_w)
	{
		fill_quad_col(game, x);
		x++;
	}
	draw_minimap(game, &scr);
	quad_flush(game, &scr);
}
