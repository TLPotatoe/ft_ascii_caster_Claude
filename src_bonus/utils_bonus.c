#include "ft_ascii_caster_bonus.h"
#include <stdlib.h>
#include <unistd.h>

size_t	ft_strlen(const char *s)
{
	size_t	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

void	ft_putstr_fd(const char *s, int fd)
{
	if (!s)
		return ;
	write(fd, s, ft_strlen(s));
}

void	free_game(t_game *game)
{
	int	i;

	if (game->map.grid)
	{
		i = 0;
		while (i < game->map.height)
		{
			free(game->map.grid[i]);
			i++;
		}
		free(game->map.grid);
		game->map.grid = NULL;
	}
	if (game->frame)
	{
		free(game->frame);
		game->frame = NULL;
	}
	free(game->screen);
	free(game->band);
	game->screen = NULL;
	game->band = NULL;
}

/* Affiche "Error\n" + message, libère tout, restaure le terminal, quitte. */
void	error_exit(t_game *game, const char *msg)
{
	ft_putstr_fd("Error\n", 2);
	ft_putstr_fd(msg, 2);
	ft_putstr_fd("\n", 2);
	if (game)
	{
		term_restore(game);
		free_game(game);
	}
	exit(1);
}

/* Valeur absolue (fabs n'est pas dans les fonctions autorisées). */
double	d_abs(double v)
{
	if (v < 0)
		return (-v);
	return (v);
}
