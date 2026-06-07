#include "ft_ascii_caster.h"
#include <stdlib.h>

/* Définit position et vecteurs (dir + plan caméra) selon le caractère joueur.
   Repère : x = colonne, y = ligne (vers le bas). Plan perpendiculaire à dir,
   de longueur PLANE_LEN (FOV ~60°). */
int	set_player(t_game *game, int x, int y, char c)
{
	game->pos_x = x + 0.5;
	game->pos_y = y + 0.5;
	if (c == 'N')
	{
		game->dir_x = 0;
		game->dir_y = -1;
	}
	else if (c == 'S')
	{
		game->dir_x = 0;
		game->dir_y = 1;
	}
	else if (c == 'E')
	{
		game->dir_x = 1;
		game->dir_y = 0;
	}
	else
	{
		game->dir_x = -1;
		game->dir_y = 0;
	}
	game->plane_x = -game->dir_y * PLANE_LEN;
	game->plane_y = game->dir_x * PLANE_LEN;
	return (0);
}

/* Vérifie charset, absence de lignes vides, et compte/place le joueur.
   Renvoie le nombre de joueurs trouvés, ou -1 si caractère/ligne invalide. */
static int	scan_grid(t_game *game)
{
	int		y;
	int		x;
	int		players;
	char	c;

	players = 0;
	y = 0;
	while (y < game->map.height)
	{
		if (game->map.grid[y][0] == '\0')
			return (-1);
		x = 0;
		while ((c = game->map.grid[y][x]) != '\0')
		{
			if (c == 'N' || c == 'S' || c == 'E' || c == 'W')
			{
				players++;
				set_player(game, x, y, c);
			}
			else if (c != '0' && c != '1')
				return (-1);
			x++;
		}
		y++;
	}
	return (players);
}

/* Renvoie le caractère en (x,y), ou '\0' si hors carte (= fuite vers extérieur). */
static char	cell_at(t_game *game, int x, int y)
{
	if (y < 0 || y >= game->map.height || x < 0)
		return ('\0');
	if (x >= (int)ft_strlen(game->map.grid[y]))
		return ('\0');
	return (game->map.grid[y][x]);
}

/* Examine les 4 voisins de la cellule cur. Empile les cellules franchissables
   non visitées. Renvoie 0 si un voisin fuit hors carte (carte non close). */
static int	flood_step(t_game *g, char *vis, int *st, int *top, int cur)
{
	int		x;
	int		y;
	int		k;
	int		w;
	const int	d[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
	char	c;

	w = g->map.width;
	x = cur % w;
	y = cur / w;
	k = 0;
	while (k < 4)
	{
		c = cell_at(g, x + d[k][0], y + d[k][1]);
		if (c == '\0')
			return (0);
		if (c != '1' && !vis[(y + d[k][1]) * w + (x + d[k][0])])
			st[(*top)++] = (y + d[k][1]) * w + (x + d[k][0]);
		k++;
	}
	return (1);
}

/* Flood-fill itératif depuis le joueur. Renvoie 1 si la zone est fermée par
   des murs, 0 si elle fuit vers l'extérieur (bord non clos). */
static int	is_closed(t_game *game)
{
	char	*visited;
	int		*stack;
	int		top;
	int		i;
	int		cur;

	visited = malloc((size_t)(game->map.width * game->map.height));
	stack = malloc(sizeof(int) * (size_t)(game->map.width * game->map.height) * 4);
	if (!visited || !stack)
		return (free(visited), free(stack), error_exit(game, "alloc"), 0);
	i = -1;
	while (++i < game->map.width * game->map.height)
		visited[i] = 0;
	top = 0;
	stack[top++] = (int)game->pos_y * game->map.width + (int)game->pos_x;
	while (top > 0)
	{
		cur = stack[--top];
		if (visited[cur])
			continue ;
		visited[cur] = 1;
		if (!flood_step(game, visited, stack, &top, cur))
			return (free(visited), free(stack), 0);
	}
	return (free(visited), free(stack), 1);
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
	if (!is_closed(game))
		error_exit(game, "map is not closed by walls");
	return (0);
}
