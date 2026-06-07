#include "ft_ascii_caster_bonus.h"

/* Renvoie le caractère en (x,y), ou '\0' si la position est hors de la carte
   (hors grille ou au-delà d'une ligne plus courte = extérieur). */
static char	cell_at(t_game *game, int x, int y)
{
	if (y < 0 || y >= game->map.height || x < 0)
		return ('\0');
	if (x >= (int)ft_strlen(game->map.grid[y]))
		return ('\0');
	return (game->map.grid[y][x]);
}

/* Vrai si la case (x,y) a au moins un voisin hors carte : une case
   franchissable dans ce cas n'est pas fermée par des murs. */
static int	cell_leaks(t_game *game, int x, int y)
{
	if (cell_at(game, x, y - 1) == '\0' || cell_at(game, x, y + 1) == '\0')
		return (1);
	if (cell_at(game, x - 1, y) == '\0' || cell_at(game, x + 1, y) == '\0')
		return (1);
	return (0);
}

/* La carte est fermée si aucune case franchissable (0 ou joueur) ne touche
   l'extérieur. Renvoie 1 si fermée, 0 sinon. */
int	map_is_closed(t_game *game)
{
	int		y;
	int		x;
	char	c;

	y = 0;
	while (y < game->map.height)
	{
		x = 0;
		while (x < (int)ft_strlen(game->map.grid[y]))
		{
			c = game->map.grid[y][x];
			if (c != '1' && cell_leaks(game, x, y))
				return (0);
			x++;
		}
		y++;
	}
	return (1);
}
