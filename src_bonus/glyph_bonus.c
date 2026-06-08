#include "ft_ascii_caster_bonus.h"

/* Indice de face 0..3 -> lettre N/S/E/W (mode r). */
static char	face_letter(int f)
{
	if (f == 0)
		return ('N');
	if (f == 1)
		return ('S');
	if (f == 2)
		return ('E');
	return ('W');
}

/* Mode t — caractère de densité selon la distance (proche = dense -> loin =
   léger), rampe libre (aucune restriction du mandatoire). Le mur garde par
   ailleurs sa couleur de face + nuance (cf. fill_column / cell_color). */
static char	shade_char(double dist)
{
	const char	*ramp = "@#%*+=~:-.";
	int			i;

	i = (int)(dist / 1.4);
	if (i < 0)
		i = 0;
	if (i > 9)
		i = 9;
	return (ramp[i]);
}

/* Caractère d'un mur selon le mode courant : lettre de face (MODE_FACE) ou
   caractère de densité par distance (MODE_SHADE). */
char	wall_glyph(t_game *g, int f, double dist)
{
	if (g->mode == MODE_SHADE)
		return (shade_char(dist));
	return (face_letter(f));
}
