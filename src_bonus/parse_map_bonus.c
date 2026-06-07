#include "ft_ascii_caster_bonus.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

/* Compte les lignes du contenu (séparées par '\n').
   Un éventuel '\n' final ne crée pas de ligne vide supplémentaire. */
static int	count_lines(const char *s)
{
	int	count;
	int	i;

	count = 0;
	i = 0;
	while (s[i])
	{
		count++;
		while (s[i] && s[i] != '\n')
			i++;
		if (s[i] == '\n')
			i++;
	}
	return (count);
}

/* Copie une ligne (jusqu'à '\n' ou fin) dans un buffer malloc. */
static char	*dup_line(const char *s, int *idx)
{
	int		start;
	int		len;
	char	*line;
	int		i;

	start = *idx;
	while (s[*idx] && s[*idx] != '\n')
		(*idx)++;
	len = *idx - start;
	line = malloc((size_t)len + 1);
	if (!line)
		return (NULL);
	i = 0;
	while (i < len)
	{
		line[i] = s[start + i];
		i++;
	}
	line[i] = '\0';
	if (s[*idx] == '\n')
		(*idx)++;
	return (line);
}

/* Découpe le contenu en grille de lignes, calcule height et width. */
static int	split_lines(const char *content, t_game *game)
{
	int	i;
	int	idx;

	game->map.height = count_lines(content);
	if (game->map.height == 0)
		return (-1);
	game->map.grid = malloc(sizeof(char *) * (size_t)game->map.height);
	if (!game->map.grid)
		return (-1);
	i = 0;
	while (i < game->map.height)
		game->map.grid[i++] = NULL;
	idx = 0;
	i = 0;
	while (i < game->map.height)
	{
		game->map.grid[i] = dup_line(content, &idx);
		if (!game->map.grid[i])
			return (-1);
		if ((int)ft_strlen(game->map.grid[i]) > game->map.width)
			game->map.width = (int)ft_strlen(game->map.grid[i]);
		i++;
	}
	return (0);
}

/* Charge et découpe la carte. La validation stricte est faite ensuite. */
int	load_map(const char *path, t_game *game)
{
	int		fd;
	char	*content;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		error_exit(game, "cannot open map file");
	content = read_all(fd);
	close(fd);
	if (!content)
		error_exit(game, "cannot read map file");
	if (split_lines(content, game) != 0)
	{
		free(content);
		error_exit(game, "empty or unreadable map");
	}
	free(content);
	return (validate_map(game));
}
