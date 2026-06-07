#include "ft_ascii_caster_bonus.h"
#include <stdlib.h>

/* Définit position et vecteurs (dir + plan caméra) selon le caractère joueur.
   Repère : x = colonne, y = ligne (vers le bas). Plan perpendiculaire à dir,
   de longueur PLANE_LEN (FOV ~60°). */
static void	set_player(t_game *game, int x, int y, char c)
{
	game->pos_x = x + 0.5;
	game->pos_y = y + 0.5;
	game->dir_x = 0;
	game->dir_y = 0;
	if (c == 'N')
		game->dir_y = -1;
	else if (c == 'S')
		game->dir_y = 1;
	else if (c == 'E')
		game->dir_x = 1;
	else
		game->dir_x = -1;
	game->plane_x = -game->dir_y * PLANE_LEN;
	game->plane_y = game->dir_x * PLANE_LEN;
}

/* Parcourt une ligne : valide le charset et place le joueur le cas échéant.
   Renvoie -1 si un caractère est invalide, 0 sinon. */
static int	scan_row(t_game *game, int y, int *players)
{
	int		x;
	char	c;

	x = 0;
	c = game->map.grid[y][x];
	while (c != '\0')
	{
		if (c == 'N' || c == 'S' || c == 'E' || c == 'W')
		{
			(*players)++;
			set_player(game, x, y, c);
		}
		else if (c != '0' && c != '1')
			return (-1);
		x++;
		c = game->map.grid[y][x];
	}
	return (0);
}

/* Vérifie charset + absence de lignes vides, et compte les joueurs.
   Renvoie le nombre de joueurs trouvés, ou -1 si caractère/ligne invalide. */
static int	scan_grid(t_game *game)
{
	int	y;
	int	players;

	players = 0;
	y = 0;
	while (y < game->map.height)
	{
		if (game->map.grid[y][0] == '\0')
			return (-1);
		if (scan_row(game, y, &players) < 0)
			return (-1);
		y++;
	}
	return (players);
}

int	validate_map(t_game *game)
{
	int	players;

	players = scan_grid(game);
	if (players < 0)
		error_exit(game, "invalid character or empty line in map");
	if (players == 0)
		error_exit(game, "no player start position");
	if (players > 1)
		error_exit(game, "multiple player start positions");
	if (!map_is_closed(game))
		error_exit(game, "map is not closed by walls");
	return (0);
}
