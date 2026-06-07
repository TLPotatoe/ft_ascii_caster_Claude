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

/* Initialise l'état à zéro puis alloue le buffer d'affichage. */
static void	init_game(t_game *game)
{
	game->map.grid = NULL;
	game->map.height = 0;
	game->map.width = 0;
	game->raw_active = 0;
	game->running = 1;
	game->frame = malloc((size_t)(3 + SCR_H * (SCR_W + 1)));
	if (!game->frame)
		error_exit(game, "allocation failure");
}

/* Boucle principale : lecture clavier -> rendu -> attente,
	jusqu'à la sortie. */
static void	game_loop(t_game *game)
{
	while (game->running)
	{
		handle_input(game);
		render_frame(game);
		usleep(FRAME_US);
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
	game_loop(&game);
	term_restore(&game);
	free_game(&game);
	return (0);
}
