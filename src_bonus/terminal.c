#include "ft_ascii_caster_bonus.h"
#include <unistd.h>

/* Passe le terminal en mode raw (non canonique, sans écho) et en lecture
   non-bloquante (VMIN=0, VTIME=0) afin de capturer les touches immédiatement.
   Masque le curseur. Renvoie 0 en cas de succès, -1 sinon. */
int	term_raw_mode(t_game *game)
{
	struct termios	raw;

	if (tcgetattr(0, &game->orig_term) != 0)
		return (-1);
	raw = game->orig_term;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &raw) != 0)
		return (-1);
	game->raw_active = 1;
	ft_putstr_fd("\033[?25l", 1);
	ft_putstr_fd("\033[2J", 1);
	return (0);
}

/* Restaure la configuration terminal d'origine et réaffiche le curseur.
   Sans effet si le mode raw n'a jamais été activé. */
void	term_restore(t_game *game)
{
	if (!game || !game->raw_active)
		return ;
	ft_putstr_fd("\033[?25h", 1);
	ft_putstr_fd("\033[2J\033[H", 1);
	tcsetattr(0, TCSANOW, &game->orig_term);
	game->raw_active = 0;
}
