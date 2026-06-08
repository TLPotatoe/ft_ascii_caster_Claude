#include "ft_ascii_caster_bonus.h"
#include <unistd.h>

/* Longueur du token complet en tête de p (rem octets disponibles) : 1 pour une
   touche simple, 3 pour une flèche ESC [ C/D. Renvoie 0 si la séquence est
   incomplète (ESC seul, ou ESC [ sans 3e octet) -> à compléter plus tard. */
static int	token_len(const char *p, int rem)
{
	if (p[0] != KEY_ESC)
		return (1);
	if (rem == 1)
		return (0);
	if (p[1] != '[')
		return (1);
	if (rem == 2)
		return (0);
	return (3);
}

/* Applique un token complet : flèche (len 3) -> rotation caméra ; sinon touche
   simple -> quitter (ESC/q/Ctrl-C/Ctrl-D), mode (r/t/y/u) ou déplacement. */
static void	apply_token(t_game *g, const char *p, int len)
{
	char	c;

	if (len == 3)
	{
		if (p[2] == 'C')
			rotate_cam(g, ROT_SPEED);
		else if (p[2] == 'D')
			rotate_cam(g, -ROT_SPEED);
		return ;
	}
	c = p[0];
	if (c == KEY_ESC || c == KEY_CTRL_C || c == KEY_CTRL_D || c == KEY_Q)
		g->running = 0;
	else if (c == KEY_R || c == KEY_T || c == KEY_Y || c == KEY_U)
		apply_mode(g, c);
	else
		apply_move(g, c);
}

/* Parcourt l'accumulateur, applique chaque token complet, puis décale les
   octets restants (séquence incomplète) en tête pour la frame suivante. */
static void	parse_buffer(t_game *g)
{
	int	i;
	int	k;
	int	j;

	i = 0;
	k = token_len(g->inbuf, g->inlen);
	while (i < g->inlen && k > 0)
	{
		apply_token(g, g->inbuf + i, k);
		i += k;
		k = 0;
		if (i < g->inlen)
			k = token_len(g->inbuf + i, g->inlen - i);
	}
	j = 0;
	while (i < g->inlen)
		g->inbuf[j++] = g->inbuf[i++];
	g->inlen = j;
}

/* Lit les touches disponibles (non bloquant) dans l'accumulateur et les parse.
   Renvoie 1 si des octets ont été lus (la frame doit être redessinée). Un ESC
   resté seul (sans suite) à la frame suivante = demande de sortie. */
int	handle_input(t_game *game)
{
	int	n;
	int	room;

	room = INBUF - game->inlen;
	n = 0;
	if (room > 0)
		n = (int)read(0, game->inbuf + game->inlen, (size_t)room);
	if (n > 0)
		game->inlen += n;
	parse_buffer(game);
	if (n <= 0 && game->inlen == 1 && game->inbuf[0] == KEY_ESC)
		game->running = 0;
	return (n > 0);
}
