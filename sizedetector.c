/* Banc d'essai (HORS NORME) pour trouver la bonne methode de mesure de la
 * taille du terminal en continu. Affiche uniquement quand la taille change.
 *
 *   - Methode ANSI : demande de position du curseur (DSR \033[6n). C'est la
 *     SEULE methode utilisable dans le projet (ioctl interdit).
 *   - Methode ioctl(TIOCGWINSZ) : reference de verite, pour comparer.
 *
 * Compilation : cc -Wall -Wextra -o sizedetector sizedetector.c
 * Usage       : ./sizedetector   (redimensionne la fenetre ; 'q' pour quitter)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

static struct termios	g_orig;

static void	restore_term(void)
{
	tcsetattr(0, TCSANOW, &g_orig);
}

/* Mode raw minimal : pas d'echo, pas de mode canonique (octets immediats).
 * VMIN=0/VTIME=1 -> read attend jusqu'a 0,1 s un octet puis rend la main.
 * On garde ISIG actif pour que Ctrl-C marche (atexit restaure le terminal). */
static void	set_raw(void)
{
	struct termios	raw;

	if (tcgetattr(0, &g_orig) != 0)
	{
		fprintf(stderr, "pas un terminal\n");
		exit(1);
	}
	atexit(restore_term);
	raw = g_orig;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	tcsetattr(0, TCSANOW, &raw);
}

/* METHODE ANSI (projet) : sauve le curseur, le pousse en bas a droite (le
 * terminal clampe a sa taille reelle), demande sa position (\033[6n), puis le
 * restaure. Le terminal repond "\033[<lignes>;<colonnes>R" sur stdin. */
static int	ansi_size(int *cols, int *rows)
{
	char	buf[64];
	char	*p;
	char	c;
	int		i;

	if (write(1, "\0337\033[999;999H\033[6n\0338", 18) < 0)
		return (-1);
	i = 0;
	while (i < (int)sizeof(buf) - 1)
	{
		if (read(0, &c, 1) <= 0)
			break ;
		buf[i++] = c;
		if (c == 'R')
			break ;
	}
	buf[i] = '\0';
	p = strchr(buf, '[');
	if (!p || sscanf(p + 1, "%d;%dR", rows, cols) != 2)
		return (-1);
	return (0);
}

/* REFERENCE (ioctl INTERDIT dans le projet) : la verite, pour comparer. */
static int	ioctl_size(int *cols, int *rows)
{
	struct winsize	ws;

	if (ioctl(1, TIOCGWINSZ, &ws) != 0)
		return (-1);
	*cols = ws.ws_col;
	*rows = ws.ws_row;
	return (0);
}

int	main(void)
{
	int		aw;
	int		ah;
	int		iw;
	int		ih;
	int		paw;
	int		pah;
	char	c;

	set_raw();
	paw = -1;
	pah = -1;
	printf("Detecteur de taille du terminal. Redimensionne la fenetre.\r\n");
	printf("'q' pour quitter.  (ANSI = methode du projet, ioctl = reference)\r\n");
	fflush(stdout);
	while (1)
	{
		if (read(0, &c, 1) > 0 && c == 'q')
			break ;
		if (ansi_size(&aw, &ah) == 0 && (aw != paw || ah != pah))
		{
			if (ioctl_size(&iw, &ih) != 0)
			{
				iw = -1;
				ih = -1;
			}
			printf("ANSI: %3d col x %3d lignes   |   ioctl(ref): %3d x %3d\r\n",
				aw, ah, iw, ih);
			fflush(stdout);
			paw = aw;
			pah = ah;
		}
		usleep(150000);
	}
	return (0);
}
