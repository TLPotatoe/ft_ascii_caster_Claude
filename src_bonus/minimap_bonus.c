#include "ft_ascii_caster_bonus.h"

/* Caractère de joueur selon la direction de regard dominante. */
static char	player_char(t_game *g)
{
	if (d_abs(g->dir_y) >= d_abs(g->dir_x))
	{
		if (g->dir_y < 0)
			return ('^');
		return ('v');
	}
	if (g->dir_x > 0)
		return ('>');
	return ('<');
}

/* Écrit un caractère dans la grille écran si la position est visible ; la
   mini-carte reste en pleine luminosité (palier 0), non assombrie. */
static void	put_cell(t_screen *scr, int sx, int sy, char c)
{
	if (sx < 0 || sx >= scr->w || sy < 0 || sy >= scr->h)
		return ;
	scr->ch[sy * scr->w + sx] = c;
	scr->band[sy * scr->w + sx] = 0;
}

/* Bonus — mini-carte 2D en haut à gauche : murs '#', sol '.', joueur fléché.
   Échelle 1 cellule = 1 caractère ; clippée aux bords de l'écran. */
void	draw_minimap(t_game *game, t_screen *scr)
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
			c = '.';
			if (game->map.grid[y][x] == '1')
				c = '#';
			put_cell(scr, MM_OX + x, MM_OY + y, c);
			x++;
		}
		y++;
	}
	put_cell(scr, MM_OX + (int)game->pos_x, MM_OY + (int)game->pos_y,
		player_char(game));
}
