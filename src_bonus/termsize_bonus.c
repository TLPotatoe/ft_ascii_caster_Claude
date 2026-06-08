#include "ft_ascii_caster_bonus.h"
#include <unistd.h>

/* Extrait lignes puis colonnes de "\033[<lignes>;<colonnes>R" : on accumule
   les chiffres dans la cible courante, le ';' bascule sur les colonnes. */
static void	parse_report(const char *b, int *rows, int *cols)
{
	int	i;
	int	*cur;

	i = 0;
	*rows = 0;
	*cols = 0;
	cur = rows;
	while (b[i])
	{
		if (b[i] >= '0' && b[i] <= '9')
			*cur = *cur * 10 + (b[i] - '0');
		else if (b[i] == ';')
			cur = cols;
		i++;
	}
}

/* Bascule la lecture stdin en bloquant borné (VTIME en dixièmes de seconde,
   VMIN=0) le temps de capter la réponse du terminal, comme le banc d'essai
   validé ; tenths=0 rétablit le non-bloquant utilisé en jeu. */
static void	set_read_timeout(int tenths)
{
	struct termios	t;

	if (tcgetattr(0, &t) != 0)
		return ;
	t.c_cc[VMIN] = 0;
	t.c_cc[VTIME] = (unsigned char)tenths;
	tcsetattr(0, TCSANOW, &t);
}

/* Lit la réponse octet par octet jusqu'au 'R' final (ou expiration du VTIME :
   read renvoie 0 -> on s'arrête). Renvoie le nombre d'octets lus. */
static int	read_report(char *buf, int max)
{
	int		i;
	char	c;

	i = 0;
	while (i < max - 1)
	{
		if (read(0, &c, 1) <= 0)
			break ;
		buf[i++] = c;
		if (c == 'R')
			break ;
	}
	buf[i] = '\0';
	return (i);
}

/* Mesure la taille du terminal sans ioctl (interdit) : sauve le curseur, le
   pousse en bas à droite (le terminal clampe), demande sa position (\033[6n)
   et le restaure ; le terminal renvoie ses dimensions sur stdin (write/read
   seuls). Renvoie 1 et remplit cols et rows si une réponse valide arrive. */
int	measure_size(int *cols, int *rows)
{
	char	buf[32];

	*rows = 0;
	*cols = 0;
	set_read_timeout(1);
	write(1, "\0337\033[999;999H\033[6n\0338", 18);
	if (read_report(buf, 32) > 0)
		parse_report(buf, rows, cols);
	set_read_timeout(0);
	return (*rows > 1 && *cols > 0);
}

/* Détection au démarrage : repli DEF_W/DEF_H si rien ne répond (non-tty). */
void	detect_screen_size(t_game *game)
{
	int	cols;
	int	rows;

	game->scr_w = DEF_W;
	game->scr_h = DEF_H;
	if (measure_size(&cols, &rows))
		apply_size(game, cols, rows);
}
