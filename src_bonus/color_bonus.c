#include "ft_ascii_caster_bonus.h"

/* Bonus couleur — distance -> palier de luminosité (0 = proche/clair,
   5 = loin/sombre). Mêmes coupures d'esprit que l'ombrage mandatoire (A6). */
int	dist_band(double dist)
{
	if (dist < 2.0)
		return (0);
	if (dist < 4.0)
		return (1);
	if (dist < 7.0)
		return (2);
	if (dist < 12.0)
		return (3);
	if (dist < 20.0)
		return (4);
	return (5);
}

/* Code couleur de cellule -> littéral ANSI (avant-plan) pour les modes
   lettres/ombrage. COL_BG (plafond/sol) -> NULL : pas d'escape, on garde la
   couleur active (espaces invisibles). COL_WHITE (mini-carte) -> blanc.
   Sinon : teinte (code / 6) x nuance (code % 6). */
const char	*cell_color(int code)
{
	if (code == COL_BG)
		return (0);
	if (code == COL_WHITE)
		return ("\033[38;5;255m");
	return (face_color(code / 6, code % 6));
}

/* Comme cell_color mais COL_BG -> avant-plan par défaut (\033[39m) : utilisé
   par le mode demi-bloc, où chaque demi-cellule a une couleur explicite. */
const char	*code_fg(int code)
{
	if (code == COL_BG)
		return ("\033[39m");
	if (code == COL_WHITE)
		return ("\033[38;5;255m");
	return (face_color(code / 6, code % 6));
}

/* Copie la chaîne s dans le buffer p, renvoie le pointeur avancé. */
char	*append_str(char *p, const char *s)
{
	while (*s)
		*p++ = *s++;
	return (p);
}

/* Émet le code couleur seulement s'il change (compression par plages), puis
   écrit le caractère. NULL (plafond/sol) -> pas d'escape, couleur conservée. */
char	*emit_cell(char *p, t_screen *s, int idx, const char **pv)
{
	const char	*col;

	col = cell_color(s->band[idx]);
	if (col && col != *pv)
	{
		p = append_str(p, col);
		*pv = col;
	}
	*p++ = s->ch[idx];
	return (p);
}
