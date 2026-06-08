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

/* Face N/S/E/W -> indice 0..3 (teinte) ; -1 si ce n'est pas une face de mur
   (caractère de mini-carte : '#', '.', flèche). */
static int	face_index(char c)
{
	if (c == 'N')
		return (0);
	if (c == 'S')
		return (1);
	if (c == 'E')
		return (2);
	if (c == 'W')
		return (3);
	return (-1);
}

/* Couleur d'une cellule = teinte (caractère de face) x nuance (palier de
   distance). La mini-carte (face_index < 0) reste en blanc. NULL pour le
   plafond/sol (BAND_SPACE) : pas d'escape, on garde la couleur courante. */
const char	*cell_color(char ch, int band)
{
	int	f;

	if (band == BAND_SPACE)
		return (0);
	f = face_index(ch);
	if (f < 0)
		return ("\033[38;5;255m");
	return (face_color(f, band));
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
char	*emit_cell(char *p, t_screen *scr, int idx, const char **prev)
{
	const char	*col;

	col = cell_color(scr->ch[idx], scr->band[idx]);
	if (col && col != *prev)
	{
		p = append_str(p, col);
		*prev = col;
	}
	*p++ = scr->ch[idx];
	return (p);
}
