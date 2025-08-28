# Guide de Présentation - Projet Malloc

## Introduction et démonstration

### Points clés à retenir pour la soutenance

🎯 **Objectif principal** : Réimplémenter `malloc`, `free`, et `realloc` en utilisant uniquement `mmap`/`munmap`

🏗️ **Architecture innovante** : Système à 3 zones (TINY/SMALL/LARGE) optimisant l'usage mémoire

⚡ **Performance** : Minimisation des appels système via zones pré-allouées

🔒 **Robustesse** : Thread safety et fonctionnalités debug avancées

---

## Plan de présentation (15-20 minutes)

### 1. Introduction (2-3 minutes)
```
"Bonjour, je vais présenter mon implémentation du projet malloc.

L'objectif était de recréer les fonctions d'allocation mémoire de la libc
en utilisant uniquement mmap et munmap, avec une architecture optimisée
pour différentes tailles d'allocation."
```

**Montrer rapidement:**
```bash
ls -la # Montrer les fichiers du projet
make   # Compilation rapide
./run_linux.sh ./test0 # Test basique fonctionnel
```

### 2. Architecture générale (5-6 minutes)

**Schema à présenter:**
```
┌─────────────────────────────────────────────────────────┐
│                  Architecture malloc                   │
├─────────────────┬─────────────────┬─────────────────────┤
│  TINY zones     │  SMALL zones    │  LARGE blocks       │
│  (≤ 512 bytes)  │  (≤ 4096 bytes) │  (> 4096 bytes)     │
│  Zone: 64KB     │  Zone: 512KB    │  Individual mmap    │
├─────────────────┼─────────────────┼─────────────────────┤
│ 100+ allocations│ 100+ allocations│ 1 alloc = 1 mmap    │
└─────────────────┴─────────────────┴─────────────────────┘
```

**Points importants:**
- **Optimisation mémoire** : 1 mmap pour 100+ allocations (TINY/SMALL)
- **Réduction fragmentation** : Split et merge des blocs
- **Simplicité** : LARGE direct en mmap individuel
- **Alignement** : 16 bytes pour optimisations processeur

**Démonstration:**
```bash
# Montrer la visualisation mémoire
export MALLOC_STACK_LOGGING=1
./run_linux.sh ./test_bonus
```

### 3. Implémentation technique (4-5 minutes)

**Structures de données clés:**
```c
// Heap global avec thread safety
typedef struct s_heap {
    t_zone              *tiny;
    t_zone              *small;  
    t_block             *large;
    pthread_mutex_t     mutex;    // Thread safety
    t_debug_flags       debug;    // Bonus
    t_alloc_history     *history; // Bonus
} t_heap;
```

**Algorithme malloc simplifié:**
```c
void *malloc(size_t size) {
    size = ALIGN(size);
    pthread_mutex_lock(&mutex);
    
    if (size ≤ 512)        → allocate_from_zone(tiny)
    else if (size ≤ 4096)  → allocate_from_zone(small)  
    else                   → allocate_large()
    
    pthread_mutex_unlock(&mutex);
}
```

**Points techniques:**
- **Thread safety** : mutex global sur toutes les opérations
- **Gestion erreurs** : retour NULL, pas de segfault
- **Optimisation** : réutilisation blocs libérés

### 4. Fonctionnalités bonus (4-5 minutes)

**Variables d'environnement implémentées:**
```bash
# Debug memory scribbling
export MALLOC_SCRIBBLE=1      # 0xDE sur free
export MALLOC_PRE_SCRIBBLE=1  # 0xAA sur malloc

# Historique allocations  
export MALLOC_STACK_LOGGING=1

# Niveau de vérification
export MALLOC_CHECK_=2
```

**Démonstration live:**
```bash
# Sans debug
./run_linux.sh ./test_env_demo

# Avec debug complet
MALLOC_SCRIBBLE=1 MALLOC_PRE_SCRIBBLE=1 MALLOC_STACK_LOGGING=1 \
./run_linux.sh ./test_env_demo
```

**show_alloc_mem_ex() avancée:**
- Configuration debug affichée
- Historique des 20 dernières allocations
- Hexdump des zones mémoire  
- Timestamps d'allocation

**Défragmentation:**
```c
malloc_defragment(); // API publique pour fusion blocs libres
```

### 5. Tests et validation (2-3 minutes)

**Tests disponibles:**
```bash
make test              # Suite de tests unitaires
make test-debug        # Avec sanitizers
make test-valgrind     # Avec Valgrind
./demo_bonus.sh        # Démonstration bonus
```

**Tests de correction classiques:**
```bash
export HOSTTYPE=Testing
make re
./run_linux.sh /usr/bin/time -v ./test0
./run_linux.sh /usr/bin/time -v ./test1
```

**Robustesse:**
- malloc(0) géré
- free(NULL) géré  
- Pas de double free
- Thread safety validé
- Tests de stress multi-thread

---

## Questions fréquentes et réponses

### Q: "Pourquoi un mutex global plutôt que des mutex per-zone ?"
**R:** "Simplicité d'implémentation et garantie de cohérence. Un mutex per-zone serait plus performant mais complexifierait la gestion des allocations croisées zones et la défragmentation."

### Q: "Comment gérez-vous la fragmentation ?"
**R:** "Triple approche : split des blocs trop grands, merge des blocs libres adjacents, et fonction malloc_defragment() publique pour forcer la défragmentation."

### Q: "Vos performances vs malloc système ?"
**R:** "Plus lent à cause du mutex global et de la gestion manuelle, mais optimisé pour réduire les appels mmap. L'objectif était l'apprentissage, pas la performance pure."

### Q: "Thread safety : comment l'avez-vous testé ?"
**R:** "Tests multi-thread avec 8 threads faisant chacun 1000 allocations/libérations simultanées. Aucun crash ou corruption détectée."

### Q: "Que se passe-t-il si mmap échoue ?"
**R:** "Retour NULL comme malloc système. Gestion propre des erreurs sans segfault."

---

## Démonstration pratique recommandée

### Demo 1: Fonctionnement basique (1 min)
```bash
make && ./run_linux.sh ./test0
```

### Demo 2: Visualisation mémoire (2 min)  
```bash
MALLOC_STACK_LOGGING=1 ./run_linux.sh ./test_bonus
# Montrer l'historique et les zones
```

### Demo 3: Variables debug (2 min)
```bash
# Montrer scribbling
MALLOC_SCRIBBLE=1 MALLOC_PRE_SCRIBBLE=1 ./run_linux.sh ./test_env_demo
# Pointer sur les 0xDE et 0xAA dans le hexdump
```

### Demo 4: Thread safety (1 min)
```bash
./run_linux.sh ./test_multithread
# Montrer que ça ne crash pas
```

---

## Points forts à mettre en avant

✅ **Conformité complète** au sujet (partie obligatoire + bonus)

✅ **Architecture réfléchie** : zones optimisées selon tailles

✅ **Code propre** : structures claires, gestion d'erreurs, documentation

✅ **Fonctionnalités bonus complètes** :
   - Thread safety natif
   - Variables debug comme malloc système  
   - show_alloc_mem_ex() avec hexdump
   - Défragmentation implémentée

✅ **Tests exhaustifs** : unitaires, stress, multi-thread, edge cases

✅ **Performance acceptable** : optimisations mmap, réutilisation mémoire

✅ **Robustesse** : gestion erreurs, pas de segfault, validation Valgrind

---

## Conclusion pour soutenance

```
"Cette implémentation malloc démontre une compréhension approfondie
de la gestion mémoire système. L'architecture 3-zones optimise 
l'usage mémoire tout en minimisant les appels système.

Les fonctionnalités bonus (thread safety, debug, défragmentation)  
montrent une approche professionnelle du développement système.

Le projet est entièrement fonctionnel, testé, et documenté.
Il respecte toutes les contraintes du sujet et va au-delà
avec des fonctionnalités avancées."

Score attendu : 125/100 🎉
```

---

## Conseils pour la soutenance

1. **Préparer l'environnement** : projet compilé, tests prêts
2. **Maîtriser les schemas** : pouvoir expliquer l'architecture au tableau  
3. **Connaître le code** : structures, algorithmes, optimisations
4. **Tests live** : montrer que ça marche vraiment
5. **Assumer les choix** : expliquer pourquoi mutex global, etc.
6. **Montrer les bonus** : variables debug, hexdump, thread safety

**Temps recommandé : 15-20 minutes + questions**

Bonne chance pour ta soutenance ! 🚀