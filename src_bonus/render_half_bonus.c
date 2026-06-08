#include "ft_ascii_caster_bonus.h"

/* Code couleur d'un pixel vertical p : code du mur si p est dans [start, end],
   sinon COL_BG (plafond/sol). */
static int	cell_pix(int p, int start, int end, int code)
{
	if (p >= start && p <= end)
		return (code);
	return (COL_BG);
}

/* Une cellule = 2 pixels verticaux (top, bot) -> caractère + couleurs :
   '\1' = ▀ (demi-bloc haut, avant-plan = top), '\2' = ▄ (demi-bloc bas), ' ' =
   vide. band = couleur d'avant-plan, band2 = couleur d'arrière-plan. */
static void	set_half(t_screen *scr, int idx, int top, int bot)
{
	scr->ch[idx] = 1;
	scr->band[idx] = top;
	scr->band2[idx] = bot;
	if (top == COL_BG && bot == COL_BG)
		scr->ch[idx] = ' ';
	else if (top == COL_BG)
	{
		scr->ch[idx] = 2;
		scr->band[idx] = bot;
		scr->band2[idx] = COL_BG;
	}
}

/* Remplit une colonne en double résolution verticale (2*scr_h pixels) : pour
   chaque cellule, le pixel pair et le pixel impair forment un demi-bloc. */
static void	fill_half_column(t_game *g, t_screen *scr, int sx)
{
	t_ray	r;
	int		code;
	int		start;
	int		end;
	int		y;

	ray_setup(g, &r, 2.0 * sx / (double)g->scr_w - 1.0);
	ray_step(g, &r);
	ray_cast(g, &r);
	code = ray_face(&r) * 6 + dist_band(r.dist);
	column_height(&r, 2 * g->scr_h, &start, &end);
	y = 0;
	while (y < g->scr_h)
	{
		set_half(scr, y * g->scr_w + sx, cell_pix(2 * y, start, end, code),
			cell_pix(2 * y + 1, start, end, code));
		y++;
	}
}

/* Mode demi-bloc (y) : ~2x la résolution verticale sans changer la taille du
   terminal, via ▀/▄ et un jeu avant-plan/arrière-plan. La mini-carte est
   superposée par-dessus (glyphes pleins). */
void	render_half(t_game *game)
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
		fill_half_column(game, &scr, x);
		x++;
	}
	draw_minimap(game, &scr);
	flush_half(game, &scr);
}
