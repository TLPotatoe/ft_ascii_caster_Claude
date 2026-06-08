# ft_ascii_caster — Démarche de réalisation (vibecoding)

> Moteur de **raycasting** rendu en **ASCII** dans le terminal, en **C pur**
> (style Wolfenstein 3D / cub3d), réalisé en pilotant un agent IA (Claude Code).
> Ce document décrit **comment** le projet a été conduit (prompts utilisateur)
> et **ce que** l'IA a produit. Le journal d'ingénierie détaillé vit dans
> [`claude.md`](claude.md).

---

## 1. Méthodologie de prompting (côté utilisateur)

Le projet a été conduit en quatre temps, par des prompts successifs. Temps total
estimé : **~2 h 10**.

### 1.1 Prompt initial — cadrage & implémentation
⏱️ **~25 min mandatory + ~15 min bonus**

Le prompt suivant sert de point de départ : il fixe la source de vérité, les
règles de travail (Git, commits atomiques, tests) et impose la tenue d'un
journal d'ingénierie `claude.md`.

> « Salut.
>
> Ton objectif est de résoudre l'exercice décrit exclusivement dans le fichier
> `sujet_ft_ascii_caster.pdf`.
>
> Règles :
> 1. Considère le PDF comme l'unique source de vérité à propos de l'exercice.
> 2. N'ajoute pas de fonctionnalités qui ne sont pas demandées. Suis le sujet.
> 3. Si une partie du sujet est ambiguë, documente l'ambiguïté avant de prendre une décision.
> 4. Initialise un dépôt Git dès le début du travail.
> 5. Réalise des commits atomiques à chaque étape importante avec des messages expliquant clairement l'intention du changement.
> 6. Vérifie régulièrement ton implémentation à l'aide de tests adaptés au sujet si présent, sinon fais les tiens.
>
> Crée un fichier `claude.md` qui servira de journal d'ingénierie et qui devra
> être mis à jour tout au long du développement. Ce fichier doit contenir au
> minimum : compréhension du sujet ; contraintes identifiées ; hypothèses ; plan
> d'implémentation ; décisions techniques ; problèmes rencontrés ; solutions
> retenues ; résultats des tests ; limitations connues ; prochaines étapes.
>
> L'objectif n'est pas seulement d'obtenir une solution fonctionnelle mais aussi
> de conserver suffisamment de contexte pour qu'une nouvelle session puisse
> reprendre le travail à partir du dépôt et du fichier `claude.md`. »

### 1.2 Prompt « fuites mémoire / Ctrl-C »
⏱️ **~5 min**

Lors des expérimentations, il s'est avéré utile d'ajouter une consigne ciblée sur
la **gestion des interruptions utilisateur** et la **vérification des fuites**.

> « Attention la partie manda et bonus leak :
> Trace de la HEAP valgrind →
> `valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes -s` »

### 1.3 Prompt « mise en conformité avec la Norme 42 »
⏱️ **~10 min (manda + bonus)**

Une fois l'implémentation validée fonctionnellement, demande de mise à la Norme.

> « Très bien. Maintenant, il va falloir normer ce projet. Tu peux faire
> `norminette | grep Error` dans le workspace pour voir les erreurs. Ignore les
> erreurs "Missing or invalid 42 header". J'ai moi-même déjà changé
> l'indentation dans la plupart des fichiers pour te donner un peu d'avance.
> Garde le projet structuré. Je te conseille de renommer tous les fichiers de
> `src_bonus` avec le suffixe `_bonus` afin de pouvoir les reconnaître
> facilement en exécutant `norminette | grep Error`. Je te conseille également de
> commencer par la partie obligatoire (manda), puis de t'occuper de la partie
> bonus. »

### 1.4 Prompts « enrichissement du bonus » (post-Norme)
⏱️ **~40 min**

Une fois le projet livré et normé, une série de prompts a étendu **uniquement le
bonus** (le mandatoire reste figé sur la résolution fixe imposée). Démarche
itérative : pour la détection de taille, un **banc d'essai jetable** (hors Norme,
non conservé dans le dépôt) a d'abord servi à valider la méthode ANSI contre
`ioctl` (référence de vérité), avant de l'intégrer.

> « je voudrais que le bonus ait une résolution adaptée au terminal. Terminal
> grand = grande résolution »

> « on va faire un micro programme de test sans norme juste pour qu'on construise
> le détecteur de taille du terminal […] le but est de trouver la bonne méthode
> pour mesurer la taille du terminal »

> « Est-ce qu'on testerait pas de gérer le resize midrun »

> « J'aimerais ajouter des couleurs distinguables par textures tout en gardant
> les nuances de distances. »

> « quand j'appuie sur r → mode actuel (N/W/E/S en couleur) ; sur t → ancien mode
> mandatory sans restriction de caractères (en gardant les couleurs/gradient) ;
> sur y → un mode résolution avec ▄ █ ▌ et un jeu background/foreground pour
> doubler la résolution sans changer celle du terminal. »

> « pour le mode y, des caractères/une méthode pour réduire l'écart entre les
> pixels ? » → ajout d'un mode **`u` quadrants** (2×2 sous-pixels par cellule,
> double les deux axes, glyphe choisi par couverture). Cf. discussion dans
> [`claude.md`](claude.md) §9bis.

> « attendre une entrée utilisateur avant d'update la frame, pas d'entrée = pas
> de changement, pas besoin d'update. » → rendu **paresseux** : la frame n'est
> resérialisée et réécrite que sur entrée clavier ou resize.

> « sous lag, en maintenant ←, le joueur dérive à droite et tourne en rond,
> pourquoi ? » → bug de **désynchronisation du flux clavier** (la queue d'une
> flèche relue comme un strafe) ; corrigé par un parsing par tokens complets +
> accumulateur (`input_bonus.c`).

> « j'aimerais que la caméra soit alignée avec le monde ; de combien de degrés
> tourne-t-elle ? » → le pas de rotation (3.44°) ne divisait pas 90° ; réglé à
> **3.6° (= π/50)** côté bonus, soit 25 crans pile pour un quart de tour.

Résultat (détaillé dans [`claude.md`](claude.md) §9bis) :
- **Résolution adaptée au terminal** sans `ioctl` (interdit) : mesure via la
  *réponse de position du curseur* ANSI (`\033[6n`), lue avec `write`/`read`.
- **Redimensionnement en cours de jeu** par polling (~0,5 s), faute de `SIGWINCH`
  (`signal()` interdit) → réallocation des buffers à la volée.
- **Couleur par face + nuance de distance** : une teinte par texture
  (N rouge, S vert, E bleu, W jaune), déclinée en 6 nuances de profondeur (ANSI
  256 couleurs, littéraux fixes).
- **Quatre modes de rendu commutables** (`r`/`t`/`y`/`u`) : lettres colorées,
  densité colorée (rampe libre), **demi-bloc** `▀`/`▄` qui double la résolution
  verticale via un jeu avant-plan/arrière-plan, et **quadrants** `▖▗▘▝▌▐▀▄█…` qui
  doublent les deux axes (2×2 sous-pixels par cellule, glyphe par couverture),
  tout cela sans changer la taille du terminal.
- **Rendu paresseux** : la frame n'est resérialisée/réécrite que sur entrée
  clavier ou resize — sans entrée, rien ne change donc rien n'est redessiné.

---

## 2. Partie IA — ce que l'agent a réalisé

### 2.1 Résultat

Un projet **complet et vérifié**, livrant deux binaires :

| Cible | Contenu | Build `-Wall -Wextra -Werror` | Norminette¹ | Valgrind |
|---|---|---|---|---|
| `make` | Partie **mandatoire** | 0 warning | 0 erreur | 0 octet à la sortie |
| `make bonus` | Partie **bonus** | 0 warning | 0 erreur | 0 octet à la sortie |

¹ *hors « Missing or invalid 42 header », explicitement ignoré.*

### 2.2 Démarche suivie

1. **Lecture du sujet** (PDF unique source de vérité), extraction des contraintes
   et des points ambigus.
2. **Cadrage documenté** : 8 ambiguïtés tranchées et justifiées dans `claude.md`
   (résolution, touche de sortie, FOV, ombrage…) avant d'écrire du code.
3. **Implémentation mandatoire par étapes**, en commits atomiques : parsing strict
   → mode terminal raw non-bloquant → moteur DDA → rendu ASCII → boucle de jeu.
4. **Bonus** : collisions, textures par face, mini-carte (binaire séparé).
5. **Durcissement mémoire** : interception de `Ctrl-C`/`Ctrl-D` pour libérer
   proprement sur interruption.
6. **Mise à la Norme 42** : refactorisation (découpe en fichiers, suppression des
   `for`/affectations en condition) sans changer le comportement.

### 2.3 Décisions techniques marquantes

- **Résolution fixe 80×40 (mandatoire)** : lire la taille réelle du terminal
  exigerait `ioctl`, hors fonctions autorisées — le sujet autorise une résolution
  par défaut. Aucune fonction interdite utilisée (pas de `ioctl`/`fcntl`/`ncurses`).
- **Résolution dynamique (bonus)** : la taille du terminal est mesurée **sans
  `ioctl`** via la réponse de position du curseur ANSI (`\033[6n`, lue par
  `write`/`read`), suivie en cours de jeu par polling — terminal grand ⇒ grande
  résolution. Repli sur 80×40 hors tty.
- **Lecture non bloquante sans `fcntl`** : `termios` avec `VMIN=0`/`VTIME=0`.
- **Sortie propre sur interruption** : `signal()` étant interdit, `ISIG` est
  désactivé pour que `Ctrl-C` arrive comme un octet, traité comme une demande de
  sortie → libération complète (corrige des blocs `still reachable`).
- **Contrôle de carte fermée** : vérification **locale** (une case franchissable
  touchant l'extérieur ⇒ carte non close), sans `malloc` — choisie lors de la
  mise à la Norme à la place d'un flood-fill, pour une sémantique identique mais
  plus simple.
- **Moteur DDA** isolé dans une structure `t_ray`, séparé de l'affichage.

### 2.4 Vérifications

- **Tests de parsing** : `tests/run_tests.sh` → **9/9** (aucun argument, mauvaise
  extension, fichier introuvable, carte non fermée, caractère invalide, joueurs
  multiples, aucun joueur, ligne vide au milieu).
- **Valgrind** sur tous les chemins d'erreur **et** sur une session de jeu
  complète (via pseudo-terminal `script`) : **0 octet en usage à la sortie**,
  aucune erreur mémoire, sur les deux binaires.
- **Rendu** vérifié visuellement (perspective 3D, ombrage par distance côté
  mandatoire ; faces `N/S/E/W` colorées par texture avec nuance de distance,
  mini-carte et résolution adaptée au terminal côté bonus).

### 2.5 Structure du dépôt

```
ft_ascii_caster/
├── Makefile                 # make (manda) · make bonus
├── claude.md                # journal d'ingénierie détaillé
├── README.md                # ce document
├── includes/
│   ├── ft_ascii_caster.h
│   └── ft_ascii_caster_bonus.h
├── maps/classic.map
├── src/                     # mandatoire (DDA via t_ray, rendu, parsing…)
├── src_bonus/               # bonus, fichiers suffixés _bonus
│                            #   termsize/resize : résolution adaptée au terminal
│                            #   color/palette   : couleur par face + distance
│                            #   glyph/render_half/halfflush : modes r/t/y
│                            #   render_quad/quadflush : mode u (quadrants)
└── tests/                   # cartes invalides + run_tests.sh
```

### 2.6 Lancer le projet

```sh
make            && ./ft_ascii_caster maps/classic.map         # nécessite un vrai terminal
make bonus      && ./ft_ascii_caster_bonus maps/classic.map
sh tests/run_tests.sh                                          # tests de parsing
```

**Contrôles** : `W`/`A`/`S`/`D` se déplacer · flèches `←`/`→` pivoter ·
`q` / `Échap` / `Ctrl-C` quitter. **Bonus — modes de rendu** : `r` lettres
colorées · `t` densité colorée · `y` demi-bloc (≈2× la résolution verticale) ·
`u` quadrants (≈2× les deux axes).

---

*Le détail complet (compréhension, contraintes, hypothèses, problèmes/solutions,
résultats, limitations, prochaines étapes) est tenu à jour dans*
[`claude.md`](claude.md).
