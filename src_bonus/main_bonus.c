#include "ft_ascii_caster_bonus.h"
#include <stdlib.h>
#include <unistd.h>

/* Vérifie que le chemin se termine par l'extension ".map". */
static int	has_map_ext(const char *path)
{
	size_t	len;

	len = ft_strlen(path);
	if (len < 4)
		return (0);
	return (path[len - 4] == '.' && path[len - 3] == 'm' && path[len - 2] == 'a'
		&& path[len - 1] == 'p');
}

/* Initialise l'état à zéro ; les buffers sont alloués après la détection de la
   taille du terminal (cf. realloc_buffers dans resize_bonus.c). */
static void	init_game(t_game *game)
{
	game->map.grid = NULL;
	game->map.height = 0;
	game->map.width = 0;
	game->raw_active = 0;
	game->running = 1;
	game->scr_w = DEF_W;
	game->scr_h = DEF_H;
	game->mode = MODE_FACE;
	game->frame = NULL;
	game->screen = NULL;
	game->band = NULL;
	game->band2 = NULL;
}

/* Boucle principale : lecture clavier -> (re-mesure périodique de la taille du
	terminal) -> rendu -> attente, jusqu'à la sortie. */
static void	game_loop(t_game *game)
{
	int	tick;

	tick = 0;
	while (game->running)
	{
		handle_input(game);
		if (tick == 0)
			handle_resize(game);
		render_frame(game);
		usleep(FRAME_US);
		tick++;
		if (tick >= RESIZE_POLL)
			tick = 0;
	}
}

int	main(int argc, char **argv)
{
	t_game	game;

	if (argc != 2)
	{
		ft_putstr_fd("Error\nusage: ./ft_ascii_caster <map.map>\n", 2);
		return (1);
	}
	init_game(&game);
	if (!has_map_ext(argv[1]))
		error_exit(&game, "map file must end with .map");
	load_map(argv[1], &game);
	if (term_raw_mode(&game) != 0)
		error_exit(&game, "cannot set terminal to raw mode");
	detect_screen_size(&game);
	realloc_buffers(&game);
	game_loop(&game);
	term_restore(&game);
	free_game(&game);
	return (0);
}
