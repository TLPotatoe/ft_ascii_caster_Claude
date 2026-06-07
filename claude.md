# Journal d'ingénierie — ft_ascii_caster

> Ce fichier est le journal de bord du projet. Il doit permettre à une nouvelle
> session de reprendre le travail à partir du dépôt Git seul.
> **Source de vérité unique : `sujet_ft_ascii_caster.pdf`.**

---

## 1. Compréhension du sujet

Réaliser un **moteur de rendu 3D en mode texte (ASCII)** fonctionnant dans un
terminal UNIX, en **C pur**, via la technique du **Raycasting** (style
Wolfenstein 3D / cub3d). Le programme :

- charge une carte depuis un fichier `.map` passé en argument ;
- place un joueur et gère ses déplacements en temps réel (clavier non-bloquant) ;
- affiche une perspective 3D en ASCII via `write`/`printf` et des codes ANSI ;
- n'utilise **aucune** bibliothèque graphique externe.

Invocation : `./ft_ascii_caster maps/classic.map`

### Carte (.map)
Grille vue du dessus. Caractères autorisés :
- `1` : mur
- `0` : sol (case marchable)
- `N` / `S` / `E` / `W` : position initiale du joueur + direction de regard.

Exemple valide donné par le sujet :
```
1111111111
1000000001
1011001101
10000N0001
1111111111
```

### Boucle de rendu (par itération)
1. Récupérer la taille du terminal **ou** fixer une résolution par défaut (ex. 80×40).
2. Pour chaque colonne écran : lancer un rayon depuis le joueur, angle fonction du FOV (~60°).
3. Calculer l'intersection avec le premier mur `1` via **DDA** (Digital Differential Analysis).
4. Déduire la hauteur apparente du mur depuis la distance (effet perspective).
5. **Ombrage ASCII** : caractère fonction de la distance (proche = dense :
   `@`, puis `#`, `O`, `x`, `.` pour le lointain). Vide au-dessus/dessous = plafond/sol.

---

## 2. Contraintes identifiées

- **Langage C uniquement.**
- Compilation **`-Wall -Wextra -Werror`** sans warning.
- **Zéro fuite mémoire** : tout `malloc` libéré, y compris sur erreur/interruption.
- **Fonctions autorisées (liste fermée)** :
  `open, close, read, write, malloc, free, perror, strerror, exit, usleep`,
  les fonctions de `<math.h>` (`sin, cos, tan, sqrt`) et de `<termios.h>`.
- Bibliothèques graphiques (MiniLibX, OpenGL, **ncurses**) **interdites**.
- Rendu via écritures standard (`write` ou `printf`) + codes d'échappement ANSI.
- Binaire nommé `ft_ascii_caster`, **un seul** argument (chemin `.map`).
- Parser **intraitable** : carte non fermée par des murs, joueurs multiples,
  caractère invalide, ligne vide au milieu → `Error\n` + message explicite,
  tout libérer, quitter proprement.
- Terminal en mode **raw / non-canonique** (`struct termios`) + lecture
  **non-bloquante**, capture immédiate des touches (sans `Entrée`).

---

## 3. Hypothèses & ambiguïtés (tranchées par défaut)

| # | Ambiguïté | Décision retenue | Justification |
|---|-----------|------------------|---------------|
| A1 | Lire la taille réelle du terminal nécessite `ioctl(TIOCGWINSZ)`, **non autorisé** | Résolution **fixe 80×40** | Le sujet autorise explicitement « fixer une résolution par défaut (80×40) ». Évite une fonction interdite. |
| A2 | Touche pour quitter non spécifiée | `ESC`, `q`, ainsi que `Ctrl-C`/`Ctrl-D` quittent proprement | Convention terminal ; `Ctrl-C`/`Ctrl-D` interceptés pour libérer la mémoire sur interruption (cf. §6). |
| A3 | « flèches ou touches de rotation » | Flèches **gauche/droite** pour pivoter | Couvre le cas le plus naturel ; WASD réservé au déplacement. |
| A4 | Rôle de A/D (WASD) | A/D = **strafe** gauche/droite, W/S = avant/arrière | Mapping FPS standard ; rotation via flèches (A3). |
| A5 | FOV exact | **60°** (plane = tan(30°)·dir ≈ 0.577) | Sujet : « FOV de ~60° ». |
| A6 | Seuils d'ombrage par distance | `@`<2, `#`<4, `O`<7, `x`<12, sinon `.` | Le sujet donne l'ordre des caractères, pas les seuils → choix cohérent dégradé. |
| A7 | Collision dans le mandatoire | **Non** (bonus). Le joueur peut traverser. | Le sujet classe la collision en **bonus** ; ne pas l'ajouter au mandatoire (règle « ne pas ajouter de features non demandées »). |
| A8 | Format du message d'erreur | `Error\n` puis ligne descriptive sur stderr | Sujet : `Error` suivi d'un message explicite. |

---

## 4. Plan d'implémentation

1. **Squelette** : Makefile, header, dossier `maps/`, `claude.md`, `.gitignore`. ✅
2. **Parsing** : lecture fichier (`open`/`read`), validation stricte (charset,
   fermeture par murs, joueur unique, lignes vides), stockage grille + position joueur.
3. **Terminal** : passage en mode raw non-bloquant via `termios`, restauration à la sortie.
4. **Raycaster** : DDA par colonne, distance perpendiculaire, hauteur de mur.
5. **Render** : construction d'un buffer écran (frame) + ombrage ASCII + flush `write` + ANSI.
6. **Boucle principale** : input → update → render → `usleep`, sortie propre (free + restore termios).
7. **Tests** : cartes valides/invalides, script de vérif erreurs, valgrind (fuites).

Découpage en commits atomiques par étape.

---

## 5. Décisions techniques

- **Rendu** : un seul `write(1, buffer, len)` par frame (buffer pré-alloué),
  pas de `printf` → reste strictement dans `write`. Les séquences ANSI utilisées
  sont fixes (`\033[H`, `\033[2J`, `\033[?25l/h`), donc aucune conversion de
  nombre n'est nécessaire.
- **Non-bloquant** : `termios` avec `VMIN=0, VTIME=0` → `read` retourne
  immédiatement (évite `fcntl`, non autorisé).
- **Coordonnées** : grille `map[y][x]`, y vers le bas. Joueur en
  `(x=col+0.5, y=row+0.5)`. Vecteurs `dir` + `plane` (caméra) façon lodev.
- **ANSI** : `\033[2J\033[H` (clear+home) ou `\033[H` (home seul) pour rafraîchir.

---

## 6. Problèmes rencontrés

- **`ioctl` / `isatty` non autorisés** : impossible de lire la taille du terminal
  ou de tester proprement si stdin est un tty. → résolution fixe (A1) et on se
  repose sur l'échec de `tcgetattr` sur un non-terminal.
- **`fabs` / `floor` non autorisés** (hors `sin/cos/tan/sqrt`) : réimplémentés
  (`d_abs`, cast `(int)` pour le plancher sur coordonnées positives).
- **`size_t` inconnu** dans le header : ajout de `<stddef.h>`.
- **Pile du flood-fill** : une cellule peut être empilée par plusieurs voisins
  avant d'être marquée visitée → pile dimensionnée à `4 * w * h`.
- **Test du rendu sans tty** : l'environnement n'a pas de terminal interactif. Résolu
  en allouant un pseudo-terminal via `script -qec` pour capturer une frame.
- **`still reachable` à la sortie sur `Ctrl-C`** : en mode raw je désactivais
  `ICANON`/`ECHO` mais pas `ISIG`. `Ctrl-C` envoyait donc `SIGINT`, tuant le
  process au milieu de la boucle sans passer par `free_game` → grille + frame
  encore alloués (signalés `still reachable`, à considérer comme des fuites).
  `signal()`/`sigaction()` n'étant pas autorisés, solution : désactiver `ISIG`
  pour que `Ctrl-C` (0x03) et `Ctrl-D` (0x04) arrivent comme des octets, traités
  comme une demande de sortie propre → `free_game` exécuté. Vérifié : 0 octet en
  usage à la sortie. (Le test doit envoyer le `^C` *après* l'activation du mode
  raw, sinon la discipline de ligne du pty génère le signal avant.)

---

## 7. Solutions retenues

- Lecture complète du fichier en un buffer croissant (`read_all`), puis découpe
  en lignes (`split_lines`) ; un `\n` final ne crée pas de ligne vide.
- Validation en deux temps : `scan_grid` (charset, lignes vides, joueur unique)
  puis `map_is_closed` (vérification **locale** : la carte est fermée si aucune
  case franchissable `0`/joueur n'a de voisin hors carte). Remplace le flood-fill
  initial lors du passage à la Norme : sémantique identique pour « fermée par des
  murs » (un `0` au bord ⇒ non close), mais sans `malloc`/pile, et plus court.
- Mode raw : `termios` avec `ICANON`/`ECHO`/`ISIG` désactivés et `VMIN=0/VTIME=0`
  pour une lecture non bloquante sans `fcntl`. `ISIG` off → `Ctrl-C`/`Ctrl-D`
  arrivent comme des octets (`0x03`/`0x04`) interceptés par `handle_input` pour
  une sortie propre (libération sur interruption sans `signal()`).
- Rendu : DDA façon lodev, un seul `write` par frame depuis un buffer pré-alloué,
  rafraîchissement via `\033[H` (curseur en haut) ; curseur masqué pendant le jeu.
- Sortie toujours propre : `error_exit`, la fin de boucle et l'interruption
  clavier restaurent le terminal et libèrent toute la mémoire.

---

## 8. Résultats des tests

Script : `tests/run_tests.sh` → **9/9 réussis** (tous les cas d'erreur affichent
`Error` + message et sortent avec un code != 0).

Cas couverts : aucun argument, mauvaise extension, fichier introuvable, carte non
fermée, caractère invalide, joueurs multiples, aucun joueur, ligne vide au milieu.

- **Valgrind** (`--leak-check=full --show-leak-kinds=all`) sur tous les chemins
  d'erreur **et** sur une session de jeu complète (via pty), pour les deux
  binaires : **0 octet en usage à la sortie** (y compris `still reachable`),
  **aucune erreur mémoire**. Couvre les sorties `q`/`ESC` **et** `Ctrl-C`.
- **Compilation** `-Wall -Wextra -Werror` (mandatoire + bonus) : **0 warning**.
- **Rendu** vérifié via pseudo-terminal (`script`) sur `maps/classic.map` :
  mandatoire = murs perspectivés + ombrage par distance (`@`/`#`/`O`/`x`/`.`) ;
  bonus = faces `N`/`S`/`E`/`W` + mini-carte ; plafond/sol en espaces, sortie
  propre.

> Note méthodo : tester l'interruption exige d'envoyer le `^C` *après*
> l'activation du mode raw (ex. `(sleep 1; printf '\003') | script -qec ...`),
> sinon la discipline de ligne du pty génère le `SIGINT` avant que le programme
> n'ait désactivé `ISIG`.

### Mise à la Norme (norminette)

Le code respecte la **Norme 42** (`norminette`), à l'exception de l'en-tête 42
(non requis ici). Contraintes appliquées : pas de `for`, pas d'affectation en
condition, ≤ 25 lignes/fonction, ≤ 5 variables/fonction, ≤ 4 arguments,
≤ 5 fonctions/fichier, pas de commentaire dans un corps de fonction, alignement
par tabulations.

Réorganisation induite (mandatoire) :
- `parse_read.c` (lecture brute : `read_all` + `append`) séparé de `parse_map.c`
  (découpe en lignes).
- `map_closed.c` (vérif de fermeture locale) séparé de `parse_check.c`
  (charset + joueur).
- `raycaster.c` (DDA pur, via `t_ray`) séparé de `render.c` (projection,
  ombrage, sérialisation de la frame). `d_abs` déplacé dans `utils.c`.
- Structure `t_ray` ajoutée au header pour réduire le nombre de variables locales
  du moteur.

Vérif : `norminette src/ includes/ft_ascii_caster.h | grep Error` ne renvoie que
les lignes « Missing or invalid 42 header » (ignorées).

---

## 9. Limitations connues

- Taille d'affichage fixe (pas de redimensionnement dynamique du terminal) — cf. A1.
- Lecture clavier : si plusieurs octets arrivent dans un même `read` (touches très
  rapprochées), seul le premier est interprété par frame (les flèches, séquences
  de 3 octets, restent gérées). Sans impact en jeu réel (≈60 FPS). Volontaire pour
  garder un parsing d'entrée simple et sûr.
- Mandatoire : pas de collision (volontaire, cf. A7) → le joueur peut traverser
  les murs. Corrigé dans la **version bonus**.

---

## 9bis. Partie bonus

Implémentée dans **`src_bonus/`**, binaire séparé **`ft_ascii_caster_bonus`**
via **`make bonus`** (le mandatoire reste accessible via `make`, inchangé).

- **Collisions** (`src_bonus/player.c`) : déplacement testé axe par axe
  (`try_move`) → glissement le long des murs, pas d'arrêt net, pas de traversée.
- **Textures par face** (`src_bonus/raycaster.c`) : le caractère du mur dépend de
  la face touchée par le rayon — `N`/`S` (murs horizontaux selon `step` en y),
  `E`/`W` (murs verticaux selon `step` en x). Remplace l'ombrage par distance.
- **Mini-carte** (`src_bonus/minimap.c`) : vue 2D en haut-gauche, murs `#`, sol
  `.`, joueur fléché (`^ v < >`) selon l'orientation, mise à jour en temps réel,
  clippée aux bords de l'écran.

Organisation : les fichiers non modifiés (`parse_*`, `terminal`, `utils`) sont
dupliqués dans `src_bonus/` (incluant le header bonus) pour isoler totalement les
deux builds. Le header bonus `includes/ft_ascii_caster_bonus.h` reprend la même
structure + le prototype `draw_minimap`.

Vérifs bonus : compilation `-Wall -Wextra -Werror` sans warning ; valgrind propre
(parsing + session de jeu via pty) ; rendu vérifié (faces correctes + mini-carte).

---

## 10. Prochaines étapes

**Partie mandatoire : terminée et vérifiée.**

- [x] Parsing strict + tests d'erreur (9/9).
- [x] Mode terminal raw non-bloquant.
- [x] Moteur DDA + rendu ASCII.
- [x] Boucle principale + sortie propre.
- [x] Passe valgrind / vérif fuites (propre).

**Partie bonus : terminée et vérifiée** (`make bonus`).

- [x] Collisions (glissement le long des murs).
- [x] Orientation des faces (textures ASCII `N`/`S`/`E`/`W`).
- [x] Mini-carte 2D avec position/orientation du joueur.

Projet complet. Pistes éventuelles si demandé : cartes plus grandes, mini-carte
mise à l'échelle, lissage du rendu.

### Reprise rapide pour une nouvelle session
- `make` puis `./ft_ascii_caster maps/classic.map` (nécessite un vrai terminal).
- `make bonus` puis `./ft_ascii_caster_bonus maps/classic.map`.
- Tests parsing : `sh tests/run_tests.sh`.
- Rendu hors tty : `printf 'q' | script -qec "./ft_ascii_caster maps/classic.map" /dev/null`.
- Code mandatoire : `src/parse_*.c` (carte), `src/terminal.c` (raw),
  `src/raycaster.c` (DDA+rendu), `src/player.c` (input), `src/main.c` (boucle).
- Code bonus : `src_bonus/` (mêmes fichiers + `minimap.c`, collisions dans
  `player.c`, faces dans `raycaster.c`), header `includes/ft_ascii_caster_bonus.h`.
