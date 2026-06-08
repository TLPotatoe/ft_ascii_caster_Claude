#include "ft_ascii_caster_bonus.h"

/* Palettes ANSI 256 couleurs : une TEINTE par face, déclinée en 6 NUANCES de
   distance (palier 0 = proche/clair -> 5 = loin/sombre). Littéraux fixes :
   aucune conversion nombre->chaîne à l'exécution (cf. §5). */

/* Face N -> rouge. */
static const char	*red_shade(int b)
{
	if (b <= 0)
		return ("\033[38;5;196m");
	if (b == 1)
		return ("\033[38;5;160m");
	if (b == 2)
		return ("\033[38;5;124m");
	if (b == 3)
		return ("\033[38;5;88m");
	if (b == 4)
		return ("\033[38;5;52m");
	return ("\033[38;5;16m");
}

/* Face S -> vert. */
static const char	*green_shade(int b)
{
	if (b <= 0)
		return ("\033[38;5;157m");
	if (b == 1)
		return ("\033[38;5;120m");
	if (b == 2)
		return ("\033[38;5;83m");
	if (b == 3)
		return ("\033[38;5;40m");
	if (b == 4)
		return ("\033[38;5;34m");
	return ("\033[38;5;28m");
}

/* Face E -> bleu. */
static const char	*blue_shade(int b)
{
	if (b <= 0)
		return ("\033[38;5;147m");
	if (b == 1)
		return ("\033[38;5;105m");
	if (b == 2)
		return ("\033[38;5;63m");
	if (b == 3)
		return ("\033[38;5;21m");
	if (b == 4)
		return ("\033[38;5;19m");
	return ("\033[38;5;17m");
}

/* Face W -> jaune. */
static const char	*yellow_shade(int b)
{
	if (b <= 0)
		return ("\033[38;5;226m");
	if (b == 1)
		return ("\033[38;5;184m");
	if (b == 2)
		return ("\033[38;5;142m");
	if (b == 3)
		return ("\033[38;5;100m");
	if (b == 4)
		return ("\033[38;5;58m");
	return ("\033[38;5;16m");
}

/* Teinte (indice de face 0..3) x nuance (palier b) -> littéral ANSI. */
const char	*face_color(int f, int b)
{
	if (f == 0)
		return (red_shade(b));
	if (f == 1)
		return (green_shade(b));
	if (f == 2)
		return (blue_shade(b));
	return (yellow_shade(b));
}
