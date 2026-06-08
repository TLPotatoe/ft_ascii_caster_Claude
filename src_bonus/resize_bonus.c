#include "ft_ascii_caster_bonus.h"
#include <stdlib.h>
#include <unistd.h>

/* Borne la taille mesurée et la stocke. Réserve une ligne pour l'entrée
   utilisateur (visuellement) et pour absorber le '\n' de fin de rangée. */
void	apply_size(t_game *g, int cols, int rows)
{
	rows = rows - 1;
	if (cols < MIN_W)
		cols = MIN_W;
	if (cols > MAX_W)
		cols = MAX_W;
	if (rows < MIN_H)
		rows = MIN_H;
	if (rows > MAX_H)
		rows = MAX_H;
	g->scr_w = cols;
	g->scr_h = rows;
}

/* (Ré)alloue les grilles écran (caractère + 2 codes couleur pour le demi-bloc)
   et le buffer de frame. free(NULL) étant sûr, sert aussi à l'allocation
   initiale. Pire cas par cellule : CELL_MAX octets (mode demi-bloc). */
void	realloc_buffers(t_game *g)
{
	size_t	cells;

	free(g->screen);
	free(g->band);
	free(g->band2);
	free(g->frame);
	cells = (size_t)g->scr_w * (size_t)g->scr_h;
	g->screen = malloc(cells);
	g->band = malloc(cells);
	g->band2 = malloc(cells);
	g->frame = malloc((size_t)(3 + g->scr_h * (g->scr_w * CELL_MAX + 6)));
	if (!g->screen || !g->band || !g->band2 || !g->frame)
		error_exit(g, "allocation failure");
}

/* Bonus resize mid-run : re-mesure la taille ; si elle a changé, réalloue les
   buffers et nettoie l'écran. Appelé périodiquement par la boucle (pas de
   SIGWINCH, signal() interdit). Renvoie 1 si la taille a changé (la frame doit
   être redessinée), 0 si la mesure échoue ou est stable. */
int	handle_resize(t_game *g)
{
	int	cols;
	int	rows;
	int	ow;
	int	oh;

	if (!measure_size(&cols, &rows))
		return (0);
	ow = g->scr_w;
	oh = g->scr_h;
	apply_size(g, cols, rows);
	if (g->scr_w == ow && g->scr_h == oh)
		return (0);
	realloc_buffers(g);
	write(1, "\033[2J", 4);
	return (1);
}
