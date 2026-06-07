#!/bin/sh
# Tests automatisés pour ft_ascii_caster (partie parsing/erreurs).
# Le rendu interactif (mode raw) n'est pas testable sans pseudo-terminal ;
# il est vérifié manuellement via `script` (voir claude.md).

cd "$(dirname "$0")/.." || exit 1
BIN=./ft_ascii_caster
PASS=0
FAIL=0

check_error() # <map> <description>
{
	out=$("$BIN" "$1" < /dev/null 2>&1)
	code=$?
	if [ "$code" -ne 0 ] && printf '%s' "$out" | head -1 | grep -q '^Error$'; then
		echo "  OK   : $2"
		PASS=$((PASS + 1))
	else
		echo "  FAIL : $2 (exit=$code, out=$out)"
		FAIL=$((FAIL + 1))
	fi
}

echo "== Cas d'erreur (doivent afficher 'Error' et sortir != 0) =="
check_error ""                              "aucun argument est géré en amont"
check_error tests/maps/not_closed.map       "carte non fermée par des murs"
check_error tests/maps/invalid_char.map     "caractere invalide"
check_error tests/maps/two_players.map      "joueurs multiples"
check_error tests/maps/no_player.map        "aucun joueur"
check_error tests/maps/empty_line.map       "ligne vide au milieu"
check_error maps/classic.txt                "mauvaise extension"
check_error maps/inexistante.map            "fichier introuvable"

echo "== Argument manquant =="
out=$("$BIN" < /dev/null 2>&1); code=$?
if [ "$code" -ne 0 ] && printf '%s' "$out" | head -1 | grep -q '^Error$'; then
	echo "  OK   : aucun argument"; PASS=$((PASS + 1))
else
	echo "  FAIL : aucun argument"; FAIL=$((FAIL + 1))
fi

echo
echo "Résultat : $PASS réussis, $FAIL échoués"
[ "$FAIL" -eq 0 ]
