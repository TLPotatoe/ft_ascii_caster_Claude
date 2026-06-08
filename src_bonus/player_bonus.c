#include "ft_ascii_caster_bonus.h"
#include <math.h>
#include <unistd.h>

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

/* Applique une touche de déplacement WASD. */
static void	handle_move(t_game *game, char c)
{
	if (c == 'w' || c == 'W')
		player_forward(game, 1.0);
	else if (c == 's' || c == 'S')
		player_forward(game, -1.0);
	else if (c == 'a' || c == 'A')
		player_strafe(game, -1.0);
	else if (c == 'd' || c == 'D')
		player_strafe(game, 1.0);
}

/* Touches de mode de rendu : r = lettres couleur, t = densité couleur,
   y = demi-bloc (double résolution verticale), u = quadrants (double les deux
   axes). */
static void	handle_mode(t_game *game, char c)
{
	if (c == KEY_R)
		game->mode = MODE_FACE;
	else if (c == KEY_T)
		game->mode = MODE_SHADE;
	else if (c == KEY_Y)
		game->mode = MODE_HALF;
	else if (c == KEY_U)
		game->mode = MODE_QUAD;
}

/* Lit les touches disponibles (non bloquant) et met à jour l'état du jeu.
   Renvoie 1 si une touche a été lue (la frame doit être redessinée), 0 sinon
   (aucune entrée -> aucun changement, on ne redessine pas). */
int	handle_input(t_game *game)
{
	char	buf[8];
	int		n;
	char	c;

	n = (int)read(0, buf, sizeof(buf));
	if (n <= 0)
		return (0);
	if (buf[0] == KEY_ESC)
		return (handle_escape(game, buf, n), 1);
	c = buf[0];
	if (c == KEY_CTRL_C || c == KEY_CTRL_D || c == KEY_Q)
		game->running = 0;
	else if (c == KEY_R || c == KEY_T || c == KEY_Y || c == KEY_U)
		handle_mode(game, c);
	else
		handle_move(game, c);
	return (1);
}
