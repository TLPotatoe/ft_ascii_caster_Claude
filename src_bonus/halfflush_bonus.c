#include "ft_ascii_caster_bonus.h"
#include <unistd.h>

/* Recopie un littéral d'avant-plan (\033[3..m) en arrière-plan (\033[4..m) en
   inversant le '3' (indice 2) en '4'. Pas de conversion nombre->chaîne : on
   réutilise le même nombre, on ne change qu'un caractère (fidèle à §5). */
static char	*append_bg(char *p, const char *fg)
{
	int	i;

	i = 0;
	while (fg[i])
	{
		if (i == 2)
			*p++ = '4';
		else
			*p++ = fg[i];
		i++;
	}
	return (p);
}

/* Émet une cellule demi-bloc : avant-plan (band) et arrière-plan (band2) ne
   sont émis que s'ils changent ; caractère ▀ (haut) / ▄ (bas) / glyphe plein
   (mini-carte) / espace (vide). */
static char	*emit_half_cell(char *p, t_screen *scr, int idx, t_prev *pv)
{
	int	fc;
	int	bc;

	fc = scr->band[idx];
	bc = scr->band2[idx];
	if (fc != pv->fg)
	{
		p = append_str(p, code_fg(fc));
		pv->fg = fc;
	}
	if (bc != pv->bg)
	{
		p = append_bg(p, code_fg(bc));
		pv->bg = bc;
	}
	if (scr->ch[idx] == 1)
		return (append_str(p, "\342\226\200"));
	if (scr->ch[idx] == 2)
		return (append_str(p, "\342\226\204"));
	*p++ = scr->ch[idx];
	return (p);
}

/* Sérialise la grille demi-bloc (un seul write) ; reset \033[0m par rangée. */
void	flush_half(t_game *game, t_screen *scr)
{
	char	*p;
	t_prev	pv;
	int		x;
	int		y;

	p = append_str(game->frame, "\033[H");
	y = 0;
	while (y < game->scr_h)
	{
		pv.fg = -1;
		pv.bg = -1;
		x = 0;
		while (x < game->scr_w)
			p = emit_half_cell(p, scr, y * game->scr_w + x++, &pv);
		p = append_str(p, "\033[0m");
		*p++ = '\n';
		y++;
	}
	write(1, game->frame, (size_t)(p - game->frame));
}
