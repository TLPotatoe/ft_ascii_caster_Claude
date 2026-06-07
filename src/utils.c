#include "ft_ascii_caster.h"
#include <unistd.h>
#include <stdlib.h>

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

/* Écrit la représentation décimale de n dans dst, renvoie la position suivante.
   Sert à composer des séquences ANSI sans printf. */
char	*put_uint(char *dst, unsigned int n)
{
	char	tmp[10];
	int		i;

	i = 0;
	if (n == 0)
		tmp[i++] = '0';
	while (n > 0)
	{
		tmp[i++] = (char)('0' + n % 10);
		n /= 10;
	}
	while (i > 0)
		*dst++ = tmp[--i];
	return (dst);
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
