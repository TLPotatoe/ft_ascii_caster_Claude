#include "ft_ascii_caster_bonus.h"
#include <math.h>
#include <unistd.h>

/* Bonus — vrai si la case (x,y) est un mur ou hors carte. */
static int	is_blocked(t_game *g, double x, double y)
{
	int	ix;
	int	iy;

	if (x < 0 || y < 0)
		return (1);
	ix = (int)x;
	iy = (int)y;
	if (iy >= g->map.height || ix >= (int)ft_strlen(g->map.grid[iy]))
		return (1);
	return (g->map.grid[iy][ix] == '1');
}

/* Bonus — applique un déplacement avec collisions : chaque axe est tenté
   indépendamment, ce qui fait glisser le joueur le long des murs. */
static void	try_move(t_game *g, double dx, double dy)
{
	if (!is_blocked(g, g->pos_x + dx, g->pos_y))
		g->pos_x += dx;
	if (!is_blocked(g, g->pos_x, g->pos_y + dy))
		g->pos_y += dy;
}

static void	move(t_game *g, double sign)
{
	try_move(g, g->dir_x * MOVE_SPEED * sign, g->dir_y * MOVE_SPEED * sign);
}

static void	strafe(t_game *g, double sign)
{
	try_move(g, g->plane_x / PLANE_LEN * MOVE_SPEED * sign, g->plane_y
		/ PLANE_LEN * MOVE_SPEED * sign);
}

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
	if (c == KEY_CTRL_C || c == KEY_CTRL_D)
		game->running = 0;
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
