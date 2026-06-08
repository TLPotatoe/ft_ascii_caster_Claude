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
- **Pile du flood-fill** (approche initiale, **abandonnée depuis** — cf. §7) :
  une cellule pouvait être empilée par plusieurs voisins avant d'être marquée
  visitée → pile dimensionnée à `4 * w * h`. Le flood-fill a ensuite été remplacé
  par la vérification locale de `map_closed.c` (plus de `malloc`/pile).
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

- **Mandatoire** : taille d'affichage fixe 80×40 (cf. A1). La **version bonus**
  s'adapte à la taille du terminal (cf. §9bis « Résolution adaptée au terminal »)
  et **suit le redimensionnement en cours de jeu** par polling (~0,5 s), faute de
  `SIGWINCH` (cf. §9bis « Redimensionnement en cours de jeu »).
- Lecture clavier (**mandatoire**) : si plusieurs octets arrivent dans un même
  `read`, seul le premier est interprété par frame. Sous lag avec une flèche
  maintenue, la troncature d'une séquence `ESC [ C/D` désynchronise le flux et la
  queue d'une flèche (`C`/`D`) peut être relue comme un strafe → le joueur dérive.
  **Corrigé dans la version bonus** : parsing du buffer entier par tokens complets
  + accumulateur des séquences coupées (cf. §9bis « Parsing d'entrée robuste »).
  Non rétro-porté au mandatoire, figé à 80×40 (lag rare).
- Mandatoire : pas de collision (volontaire, cf. A7) → le joueur peut traverser
  les murs. Corrigé dans la **version bonus**.

---

## 9bis. Partie bonus

Implémentée dans **`src_bonus/`**, binaire séparé **`ft_ascii_caster_bonus`**
via **`make bonus`** (le mandatoire reste accessible via `make`, inchangé).

- **Collisions** (`src_bonus/move_bonus.c`) : déplacement testé axe par axe
  (`try_move`) → glissement le long des murs, pas d'arrêt net, pas de traversée.
- **Textures par face** (`src_bonus/render_bonus.c`, `ray_face`) : la face touchée
  par le rayon donne un indice 0..3 — `N`/`S` (murs horizontaux selon `step` en
  y), `E`/`W` (murs verticaux selon `step` en x) — qui pilote à la fois le
  caractère (mode lettres) et la teinte (couleur). Ordre aligné sur la palette.
- **Mini-carte** (`src_bonus/minimap_bonus.c`) : vue 2D en haut-gauche, murs `#`,
  sol `.`, joueur fléché (`^ v < >`) selon l'orientation, mise à jour temps réel,
  clippée aux bords de l'écran.
- **Résolution adaptée au terminal** (`src_bonus/termsize_bonus.c`) : le rendu
  occupe désormais toute la fenêtre — **terminal grand ⇒ grande résolution**. La
  taille est mesurée à l'exécution **sans `ioctl`** (interdit, cf. A1) via la
  *réponse de position du curseur* ANSI : on sauve le curseur (`\0337`), on le
  pousse en bas à droite (`\033[999;999H`, le terminal clampe), on demande sa
  position (`\033[6n`) et on le restaure (`\0338`) ; le terminal renvoie
  `\033[lignes;colonnesR` sur stdin — uniquement `write`/`read` (autorisés). La
  lecture bascule **temporairement** stdin en `VMIN=0`/`VTIME=1` (bloquant borné
  ~0,1 s, `set_read_timeout`) le temps de capter la réponse, puis revient en
  `VTIME=0` (non-bloquant) pour le jeu — c'est ce qui fiabilise la mesure (la
  lecture purement non-bloquante ratait la réponse). Validé d'abord sur un banc
  d'essai jetable (hors Norme, comparé à `ioctl` comme vérité ; non conservé).
  Repli **80×40** si rien ne répond (non-tty) ; bornes `MIN_*`/`MAX_*` pour
  clamper (et borner l'allocation). Une ligne est réservée (`scr_h = lignes − 1`)
  pour l'entrée utilisateur et pour absorber le `\n` de fin de rangée. Les
  dimensions deviennent des **champs runtime** de `t_game` (`scr_w`/`scr_h`) en
  remplacement des macros `SCR_W`/`SCR_H` ; la grille écran et le buffer de frame
  sont **alloués après détection** (`realloc_buffers`, libérés par `free_game`) au
  lieu d'un tableau pile de taille fixe. `t_screen` porte `w`/`h` pour la
  mini-carte.
- **Redimensionnement en cours de jeu** (`src_bonus/resize_bonus.c`) : pas de
  `SIGWINCH` possible (`signal()` interdit) → **polling**. La boucle ré-appelle
  `measure_size` tous les `RESIZE_POLL` frames (~0,5 s) ; si la taille a changé
  (`handle_resize`), on **réalloue** les buffers (`realloc_buffers`) et on nettoie
  l'écran (`\033[2J`). `apply_size` factorise le clamp + la réserve d'une ligne,
  partagé par la détection initiale et le resize. Coût : la mesure consomme les
  octets stdin en attente au moment du poll → si une touche est maintenue, un
  appui peut être avalé toutes les ~0,5 s (stutter négligeable). Note de test : le
  resize ne se vérifie qu'en **vrai terminal** (un pipe ne répond pas au `\033[6n`
  en continu, donc `measure_size` échoue et aucun resize n'est déclenché).
- **Couleur par face + nuance de distance** (`src_bonus/color_bonus.c`,
  `src_bonus/palette_bonus.c`) : chaque **face** de mur a sa **teinte**
  (N = rouge, S = vert, E = bleu, W = jaune) — textures distinguables d'un coup
  d'œil — et chaque teinte est **déclinée en 6 nuances** selon la distance
  (proche = clair → loin = sombre). Chaque cellule stocke un **code couleur** sur
  un octet (`t_screen.band`) : `0..23` pour un mur (`face*6 + dist_band`), `COL_BG`
  (30) pour le plafond/sol, `COL_WHITE` (31) pour la mini-carte. `cell_color(code)`
  traduit ce code en **littéral ANSI 256 couleurs** via `face_color` (dans
  `palette_bonus.c`, une fonction `*_shade` par couleur) ; codes ANSI **fixes**
  (suite de `if`-`return`), aucune conversion nombre→chaîne (fidèle à §5). Émis
  seulement **quand il change** (compression par plages, `emit_cell` compare le
  **pointeur** de couleur), reset `\033[0m` par rangée, **un seul `write`** par
  frame. Plafond/sol → `cell_color` renvoie `NULL` (pas d'escape, espaces
  invisibles) ; mini-carte en **blanc plein**.
- **Quatre modes de rendu** (touches `r` / `t` / `y` / `u`, champ `t_game.mode`) :
  - **`r` — lettres couleur** (défaut, `MODE_FACE`) : `N`/`S`/`E`/`W` colorés par
    teinte de face + nuance de distance (le mode décrit ci-dessus).
  - **`t` — densité couleur** (`MODE_SHADE`, `src_bonus/glyph_bonus.c`) : reprend
    l'esprit de l'ombrage mandatoire **sans restriction de caractères** (rampe
    libre `@#%*+=~:-.` choisie par distance, `shade_char`), tout en gardant la
    **couleur de face + gradient** (le caractère encode finement la distance, la
    couleur la face + le palier).
  - **`y` — demi-bloc** (`MODE_HALF`, `render_half_bonus.c` + `halfflush_bonus.c`) :
    **~2× la résolution verticale sans changer la taille du terminal**. Chaque
    cellule = **2 pixels verticaux** rendus avec `▀`/`▄` (U+2580/U+2584) et un jeu
    **avant-plan/arrière-plan** : la moitié haute prend la couleur d'avant-plan, la
    moitié basse celle d'arrière-plan. La hauteur de colonne est calculée en
    `2*scr_h` pixels ; `band` = code couleur du pixel haut, `band2` = code du pixel
    bas. Les littéraux d'arrière-plan (`\033[48;5;Nm`) sont obtenus en recopiant le
    littéral d'avant-plan en **inversant le `3` en `4`** (`append_bg`) — pas de
    nouveaux littéraux ni de conversion de nombre. La mini-carte reste superposée
    en glyphes pleins. `CELL_MAX` (26) borne le pire cas d'octets/cellule (fg + bg
    + glyphe 3 octets) pour dimensionner `frame`.
  - **`u` — quadrants** (`MODE_QUAD`, `render_quad_bonus.c` + `quadflush_bonus.c`) :
    **~2× la résolution sur les *deux* axes** sans changer la taille du terminal.
    Chaque cellule = **2×2 sous-pixels** ; l'horizontale est échantillonnée au
    double (2 sous-colonnes de rayons par cellule, `cast_sub`) et le vertical au
    double (intervalle de mur en `2*scr_h`, comme le demi-bloc). Le glyphe est
    choisi **par couverture** : un masque 4 bits (TL/TR/BL/BR, `cell_mask`) indexe
    les 16 **blocs de quadrant** Unicode (espace + `▖▗▘▝▌▐▀▄▙▟▛▜▚▞█`,
    `quad_glyph`). Avantages vs `y` : les blocs de quadrant (Block Elements) sont
    **pavés proprement** (portables), et la *négative space* du glyphe prend le
    **fond du terminal** — qui sert directement de plafond/sol, donc **aucun
    littéral d'arrière-plan à émettre** (plus simple que `y`). Couleur : teinte de
    la sous-colonne gauche si elle porte du mur, sinon droite, sinon fond
    (`put_cell`) — un quadrant étant mono-couleur, une seule teinte par cellule.
    La frame réutilise `CELL_MAX` (fg + glyphe 3 octets ≤ 14 < 26). Limite
    assumée : à une arête intérieure entre deux murs de profondeurs différentes,
    la cellule ne garde qu'une teinte. *Glyphes justifiés en discussion : `◢◣◤◥`
    (Geometric Shapes) écartés — non pavés par beaucoup de fontes, et utiles
    seulement aux arêtes diagonales alors que l'escalier d'un raycaster est
    horizontal.*
- **Rendu paresseux** (`src_bonus/main_bonus.c`, `game_loop`) : la frame n'est
  resérialisée et réécrite (`render_frame`) **que si l'état a changé** — flag
  `dirty` armé par `handle_input` (renvoie 1 si une touche a été lue) ou
  `handle_resize` (renvoie 1 si la taille a changé). Sans entrée, rien ne change
  donc rien n'est redessiné : on évite le coût de sérialisation + `write` à chaque
  frame. La boucle continue de tourner (poll clavier + `usleep`), le premier tour
  est dessiné d'office (`dirty = 1`). `handle_input`/`handle_resize` sont passés de
  `void`/`0` à un retour `int` signalant le changement.
- **Parsing d'entrée robuste** (`src_bonus/input_bonus.c`) : une flèche est une
  **séquence de 3 octets** `ESC [ C/D`. L'ancien `handle_input` lisait jusqu'à 8
  octets mais n'interprétait que `buf[0..2]` et jetait le reste. Sous lag, une
  touche maintenue accumule plusieurs séquences ; comme la fenêtre de lecture n'est
  pas un multiple de 3, une lecture coupait au milieu d'une séquence et la suivante
  démarrait désynchronisée → le 3e octet d'un `←` (`D`) était relu comme la touche
  de déplacement `D` (**strafe droite**) : la caméra tourne à gauche **et** le
  joueur dérive à droite (« tourne en rond »). Fix : `handle_input` lit dans un
  **accumulateur** (`t_game.inbuf`/`inlen`, `INBUF` octets) et `parse_buffer`
  consomme le buffer **par tokens complets** (`token_len` : 1 octet, ou 3 pour une
  flèche) via `apply_token` ; les octets d'une séquence **coupée en fin de lecture**
  sont conservés et complétés à la frame suivante — plus aucune désynchronisation.
  Un `ESC` resté seul sans suite à la frame d'après = sortie (Échap). Découpage :
  `player_bonus.c` ne garde que les *appliers* (`rotate_cam`, `apply_move`,
  `apply_mode`), le flux est parsé dans `input_bonus.c`.

Organisation (tous les fichiers bonus portent le suffixe **`_bonus`** pour les
distinguer facilement sous `norminette | grep Error`) :
- Le cœur (parsing, fermeture, DDA, terminal, utils, main) est dupliqué à
  l'identique depuis le mandatoire (seul l'include change), via les fichiers
  `*_bonus.c`. Cela isole totalement les deux builds.
- Spécifiques au bonus : `render_bonus.c` (faces), `minimap_bonus.c`,
  `move_bonus.c` (déplacement + collisions), `player_bonus.c` (rotation + input,
  avec `player_forward`/`player_strafe` exposés par le header bonus).
- Header `includes/ft_ascii_caster_bonus.h` : mêmes structures (`t_map`,
  `t_game`, `t_ray`) + prototypes bonus.

Vérifs bonus : `-Wall -Wextra -Werror` sans warning ; **norminette** sans erreur
(hors header 42) ; valgrind **0 octet en usage à la sortie** (parsing + session) ;
rendu vérifié (faces correctes + mini-carte).

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
- [x] Couleur par face + nuance de distance (teinte par texture N/S/E/W,
  déclinée en 6 nuances de profondeur ; ANSI 256 couleurs).
- [x] Quatre modes de rendu commutables (`r` lettres couleur, `t` densité couleur,
  `y` demi-bloc ~2× la résolution verticale via `▀`/`▄` + fond/avant-plan,
  `u` quadrants ~2× les deux axes via blocs de quadrant + glyphe par couverture).
- [x] Rendu paresseux : la frame n'est redessinée que sur entrée clavier/resize
  (flag `dirty` ; sans entrée, aucun redraw).
- [x] Résolution adaptée au terminal (détection ANSI sans `ioctl` ; terminal
  grand ⇒ grande résolution ; repli 80×40 hors tty).
- [x] Redimensionnement en cours de jeu (polling ~0,5 s + réallocation, sans
  `SIGWINCH`).

Projet complet. Pistes éventuelles si demandé : cartes plus grandes, mini-carte
mise à l'échelle, lissage du rendu.

### Reprise rapide pour une nouvelle session
- `make` puis `./ft_ascii_caster maps/classic.map` (nécessite un vrai terminal).
- `make bonus` puis `./ft_ascii_caster_bonus maps/classic.map`.
- Tests parsing : `sh tests/run_tests.sh`.
- Rendu hors tty : `printf 'q' | script -qec "./ft_ascii_caster maps/classic.map" /dev/null`.
- Vérif Norme : `norminette | grep Error` (ignorer « Missing or invalid 42
  header »). Pour ne voir que les vraies erreurs :
  `norminette 2>/dev/null | grep -E "^Error: " | grep -v "42 header"`.
- Code mandatoire (`src/`) : `parse_read.c`/`parse_map.c` (lecture+découpe),
  `parse_check.c` (charset/joueur), `map_closed.c` (fermeture), `terminal.c`
  (raw), `raycaster.c` (DDA via `t_ray`), `render.c` (projection/ombrage/flush),
  `player.c` (input), `utils.c`, `main.c`.
- Code bonus (`src_bonus/`, suffixe `_bonus`) : cœur dupliqué + `render_bonus.c`
  (faces + modes r/t), `glyph_bonus.c` (caractère selon le mode),
  `render_half_bonus.c` + `halfflush_bonus.c` (mode demi-bloc y),
  `render_quad_bonus.c` + `quadflush_bonus.c` (mode quadrants u),
  `color_bonus.c` + `palette_bonus.c` (couleur par face + nuance de
  distance), `minimap_bonus.c`,
  `move_bonus.c` (collisions), `player_bonus.c` (appliers rotate/move/mode) +
  `input_bonus.c` (parsing clavier par tokens + accumulateur), `termsize_bonus.c` (détection de
  la taille du terminal sans `ioctl`), `resize_bonus.c` (resize en cours de jeu
  par polling + réallocation des buffers).
  Header `includes/ft_ascii_caster_bonus.h`.
- Rendu bonus hors tty à une taille donnée (fournir la réponse curseur) :
  `printf '\033[40;120Rq' | script -qfc "./ft_ascii_caster_bonus maps/classic.map" out.txt`
  (ici 120 colonnes × 39 lignes ; sans la réponse `\033[…R`, repli 80×40).
