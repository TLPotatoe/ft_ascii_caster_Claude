#include "ft_ascii_caster.h"
#include <unistd.h>
#include <math.h>

/* Avance/recule le long du vecteur direction (signe = +1 avant, -1 arrière).
   Aucune collision dans le mandatoire (cf. ambiguïté A7). */
static void	move(t_game *g, double sign)
{
	g->pos_x += g->dir_x * MOVE_SPEED * sign;
	g->pos_y += g->dir_y * MOVE_SPEED * sign;
}

/* Déplacement latéral (strafe) perpendiculaire à la direction. */
static void	strafe(t_game *g, double sign)
{
	g->pos_x += g->plane_x / PLANE_LEN * MOVE_SPEED * sign;
	g->pos_y += g->plane_y / PLANE_LEN * MOVE_SPEED * sign;
}

/* Fait pivoter direction et plan caméra de l'angle donné (radians). */
static void	rotate(t_game *g, double a)
{
	double	odx;
	double	opx;

	odx = g->dir_x;
	g->dir_x = g->dir_x * cos(a) - g->dir_y * sin(a);
	g->dir_y = odx * sin(a) + g->dir_y * cos(a);
	opx = g->plane_x;
	g->plane_x = g->plane_x * cos(a) - g->plane_y * sin(a);
	g->plane_y = opx * sin(a) + g->plane_y * cos(a);
}

/* Interprète une séquence d'échappement (flèches) : C = droite, D = gauche. */
static void	handle_escape(t_game *g, const char *buf, int n)
{
	if (n == 1)
		g->running = 0;
	else if (n >= 3 && buf[1] == '[')
	{
		if (buf[2] == 'C')
			rotate(g, ROT_SPEED);
		else if (buf[2] == 'D')
			rotate(g, -ROT_SPEED);
	}
}

/* Lit les touches disponibles (non bloquant) et met à jour l'état du jeu. */
int	handle_input(t_game *game)
{
	char	buf[8];
	int		n;
	char	c;

	n = (int)read(0, buf, sizeof(buf));
	if (n <= 0)
		return (0);
	if (buf[0] == KEY_ESC)
		return (handle_escape(game, buf, n), 0);
	c = buf[0];
	if (c == 'w' || c == 'W')
		move(game, 1.0);
	else if (c == 's' || c == 'S')
		move(game, -1.0);
	else if (c == 'a' || c == 'A')
		strafe(game, -1.0);
	else if (c == 'd' || c == 'D')
		strafe(game, 1.0);
	else if (c == KEY_Q)
		game->running = 0;
	return (0);
}
