#include "ft_ascii_caster_bonus.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/* Lit l'intégralité d'un fd dans un buffer malloc terminé par '\0'.
   Renvoie le buffer (à free par l'appelant) ou NULL en cas d'échec. */
static char	*read_all(int fd)
{
	char	*buf;
	char	*tmp;
	char	chunk[4096];
	size_t	len;
	ssize_t	r;

	buf = malloc(1);
	if (!buf)
		return (NULL);
	buf[0] = '\0';
	len = 0;
	while ((r = read(fd, chunk, sizeof(chunk))) > 0)
	{
		tmp = malloc(len + (size_t)r + 1);
		if (!tmp)
			return (free(buf), NULL);
		for (size_t i = 0; i < len; i++)
			tmp[i] = buf[i];
		for (ssize_t i = 0; i < r; i++)
			tmp[len + (size_t)i] = chunk[i];
		len += (size_t)r;
		tmp[len] = '\0';
		free(buf);
		buf = tmp;
	}
	if (r < 0)
		return (free(buf), NULL);
	return (buf);
}

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
	int	w;

	game->map.height = count_lines(content);
	if (game->map.height == 0)
		return (-1);
	game->map.grid = malloc(sizeof(char *) * (size_t)game->map.height);
	if (!game->map.grid)
		return (-1);
	for (i = 0; i < game->map.height; i++)
		game->map.grid[i] = NULL;
	idx = 0;
	i = 0;
	while (i < game->map.height)
	{
		game->map.grid[i] = dup_line(content, &idx);
		if (!game->map.grid[i])
			return (-1);
		w = (int)ft_strlen(game->map.grid[i]);
		if (w > game->map.width)
			game->map.width = w;
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
