#!/usr/bin/env sh
# Wrapper pour exécuter une commande en préchargeant la lib malloc custom
# Usage: ./run_linux.sh <commande et args>
# Exemple: ./run_linux.sh /usr/bin/time -v ./test0

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
LIB="${SCRIPT_DIR}/libft_malloc.so"

if [ ! -f "$LIB" ]; then
  echo "[run_linux.sh] ERREUR: Bibliothèque $LIB introuvable. Construis-la avec: make" >&2
  exit 1
fi

export LD_LIBRARY_PATH="$SCRIPT_DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export LD_PRELOAD="$LIB"

exec "$@"
