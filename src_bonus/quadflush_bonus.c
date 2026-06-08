#include "ft_ascii_caster_bonus.h"
#include <unistd.h>

/* Masque 2x2 (TL=8 TR=4 BL=2 BR=1) -> bloc de quadrant Unicode (UTF-8). 0 =
   espace. Les 16 combinaisons existent en Block Elements, pavées proprement. */
static const char	*quad_glyph(int m)
{
	static const char	*g[16] = {
		" ", "\342\226\227", "\342\226\226", "\342\226\204",
		"\342\226\235", "\342\226\220", "\342\226\236", "\342\226\237",
		"\342\226\230", "\342\226\232", "\342\226\214", "\342\226\231",
		"\342\226\200", "\342\226\234", "\342\226\233", "\342\226\210"};

	return (g[m]);
}

/* Émet une cellule : couleur d'avant-plan (band) seulement si elle change, puis
   le glyphe. ch < 16 = masque de quadrant ; sinon glyphe plein (mini-carte). */
static char	*emit_quad_cell(char *p, t_screen *scr, int idx, const char **pv)
{
	const char		*col;
	unsigned char	ch;

	col = cell_color(scr->band[idx]);
	if (col && col != *pv)
	{
		p = append_str(p, col);
		*pv = col;
	}
	ch = (unsigned char)scr->ch[idx];
	if (ch < 16)
		return (append_str(p, quad_glyph(ch)));
	*p++ = (char)ch;
	return (p);
}

/* Sérialise la grille quadrants (un seul write) ; reset \033[0m par rangée. */
void	quad_flush(t_game *game, t_screen *scr)
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
			p = emit_quad_cell(p, scr, y * game->scr_w + x++, &prev);
		p = append_str(p, "\033[0m");
		*p++ = '\n';
		y++;
	}
	write(1, game->frame, (size_t)(p - game->frame));
}
