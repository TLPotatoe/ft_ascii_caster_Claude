#include "ft_ascii_caster_bonus.h"
#include <stdlib.h>
#include <unistd.h>

/* Concatène buf (len octets) et chunk (r octets) dans un nouveau buffer malloc
   terminé par '\0'. Renvoie le nouveau buffer, ou NULL en cas d'échec. */
static char	*append(char *buf, size_t len, char *chunk, ssize_t r)
{
	char	*tmp;
	size_t	i;

	tmp = malloc(len + (size_t)r + 1);
	if (!tmp)
		return (NULL);
	i = 0;
	while (i < len)
	{
		tmp[i] = buf[i];
		i++;
	}
	i = 0;
	while (i < (size_t)r)
	{
		tmp[len + i] = chunk[i];
		i++;
	}
	tmp[len + (size_t)r] = '\0';
	return (tmp);
}

/* Lit l'intégralité d'un fd dans un buffer malloc terminé par '\0'.
   Renvoie le buffer (à free par l'appelant) ou NULL en cas d'échec. */
char	*read_all(int fd)
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
	r = read(fd, chunk, sizeof(chunk));
	while (r > 0)
	{
		tmp = append(buf, len, chunk, r);
		free(buf);
		if (!tmp)
			return (NULL);
		buf = tmp;
		len += (size_t)r;
		r = read(fd, chunk, sizeof(chunk));
	}
	if (r < 0)
		return (free(buf), NULL);
	return (buf);
}
