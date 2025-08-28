# Documentation complète - Custom Malloc Implementation

## Vue d'ensemble

Cette implémentation de `malloc` reproduit le fonctionnement du système d'allocation mémoire avec une architecture à **trois zones** optimisée pour différentes tailles d'allocation.

## Architecture générale

### Principe des zones
```
┌─────────────────────────────────────────────────────────────────────┐
│                            g_heap (Global)                         │
├─────────────────┬─────────────────┬─────────────────────────────────┤
│  TINY zones     │  SMALL zones    │  LARGE blocks                   │
│  (≤ 512 bytes)  │  (≤ 4096 bytes) │  (> 4096 bytes)                 │
│  Zone: 64KB     │  Zone: 512KB    │  Individual mmap                │
├─────────────────┼─────────────────┼─────────────────────────────────┤
│ mmap() une zone │ mmap() une zone │ mmap() par allocation           │
│ Contient 100+   │ Contient 100+   │ Une allocation = un mmap        │
│ allocations     │ allocations     │                                 │
└─────────────────┴─────────────────┴─────────────────────────────────┘
```

### Structures de données principales

#### 1. t_heap - Le heap global
```c
typedef struct s_heap {
    t_zone              *tiny;        // Liste chaînée des zones TINY
    t_zone              *small;       // Liste chaînée des zones SMALL  
    t_block             *large;       // Liste chaînée des blocs LARGE
    pthread_mutex_t     mutex;        // Mutex pour thread safety
    t_debug_flags       debug;        // Flags de débogage
    t_alloc_history     *history;     // Historique des allocations
    size_t              history_count;// Nombre d'entrées dans l'historique
} t_heap;
```

#### 2. t_zone - Une zone de mémoire
```c
typedef struct s_zone {
    size_t          size;      // Taille totale de la zone
    struct s_zone   *next;     // Zone suivante dans la liste
    t_block         *blocks;   // Premier bloc dans la zone
} t_zone;
```

#### 3. t_block - Un bloc d'allocation
```c
typedef struct s_block {
    size_t          size;        // Taille du bloc utilisateur
    bool            is_free;     // Bloc libre ou occupé
    struct s_block  *next;       // Bloc suivant
    time_t          alloc_time;  // Timestamp d'allocation (bonus)
} t_block;
```

## Schéma détaillé de la mémoire

### Organisation d'une zone TINY/SMALL
```
Zone (mmap'd)
┌─────────────────────────────────────────────────────────────────────┐
│                            t_zone                                   │
│ ┌─────────────────┬────────────────────────────────────────────────┐ │
│ │ size: 65536     │ next: ptr_to_next_zone │ blocks: ptr_to_first │ │
│ └─────────────────┴────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                            t_block #1                              │
│ ┌─────────┬──────────┬──────────┬─────────────────────────────────┐ │
│ │ size:96 │ free:0   │ next:ptr │ alloc_time: 1756409765           │ │
│ └─────────┴──────────┴──────────┴─────────────────────────────────┘ │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │                    User Data (96 bytes)                        │ │
│ │                                                                 │ │
│ └─────────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                            t_block #2                              │
│ ┌─────────┬──────────┬──────────┬─────────────────────────────────┐ │
│ │ size:128│ free:1   │ next:ptr │ alloc_time: 0                   │ │
│ │         │          │          │                                 │ │
│ └─────────┴──────────┴──────────┴─────────────────────────────────┘ │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │                    Free Space (128 bytes)                      │ │
│ └─────────────────────────────────────────────────────────────────┘ │
│                              ...                                   │
└─────────────────────────────────────────────────────────────────────┘
```

### Organisation des blocs LARGE
```
Large Block (mmap'd individuellement)
┌─────────────────────────────────────────────────────────────────────┐
│                            t_block                                 │
│ ┌─────────┬──────────┬──────────┬─────────────────────────────────┐ │
│ │ size:   │ free:0   │ next:ptr │ alloc_time: 1756409765           │ │
│ │ 8192    │          │          │                                 │ │
│ └─────────┴──────────┴──────────┴─────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│                        User Data (8192 bytes)                      │
│                                                                     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Algorithmes principaux

### 1. malloc(size_t size)
```
1. Vérifier size > 0
2. Aligner size sur ALIGNMENT (16 bytes)
3. Acquérir le mutex
4. Initialiser les flags debug (première fois)
5. Router selon la taille :
   - size ≤ 512    → allocate_from_zone(tiny)
   - size ≤ 4096   → allocate_from_zone(small)  
   - size > 4096   → allocate_large()
6. Si allocation réussie :
   - Appliquer pre_scribble si activé
   - Ajouter à l'historique
7. Libérer le mutex
8. Retourner le pointeur
```

### 2. allocate_from_zone()
```
1. Si aucune zone n'existe → créer la première zone
2. Parcourir toutes les zones existantes :
   - Chercher un bloc libre suffisant
   - Si trouvé → découper si nécessaire et retourner
3. Si aucun bloc trouvé → créer une nouvelle zone
4. Allouer dans la nouvelle zone
```

### 3. free(void *ptr)
```
1. Vérifier ptr != NULL
2. Acquérir le mutex
3. Calculer l'adresse du t_block : ptr - sizeof(t_block)
4. Vérifier si c'est un bloc LARGE :
   - Si oui → unmap et retirer de la liste
5. Sinon (TINY/SMALL) :
   - Appliquer scribble si activé
   - Marquer is_free = true
   - Fusionner avec les blocs libres adjacents
6. Ajouter à l'historique
7. Libérer le mutex
```

## Fonctionnalités bonus implémentées

### 1. Thread Safety ✅
- **Mutex global** : `pthread_mutex_t mutex`
- **Protection** de toutes les opérations malloc/free/realloc
- **Sérialization** complète des accès au heap

### 2. Variables d'environnement de debug ✅

#### MALLOC_SCRIBBLE=1
```c
// Remplit la mémoire libérée avec 0xDE
if (g_heap.debug.scribble) {
    scribble_memory(user_ptr, block->size, MALLOC_SCRIBBLE_FREE);
}
```

#### MALLOC_PRE_SCRIBBLE=1  
```c
// Remplit la mémoire allouée avec 0xAA
if (g_heap.debug.pre_scribble) {
    scribble_memory(ptr, size, MALLOC_SCRIBBLE_ALLOC);
}
```

#### MALLOC_STACK_LOGGING=1
```c
// Active l'historique des allocations
typedef struct s_alloc_history {
    void            *ptr;         // Adresse allouée
    size_t          size;         // Taille
    time_t          timestamp;    // Moment de l'allocation
    bool            is_freed;     // Libéré ou non
    struct s_alloc_history *next; // Suivant
} t_alloc_history;
```

#### MALLOC_CHECK_=0-3
- **0** : Ignorer les erreurs
- **1** : Imprimer les diagnostics  
- **2** : Appeler abort() sur erreur
- **3** : abort() + diagnostic

#### MALLOC_GUARD=1
- **Placeholder** implémenté pour vérifications futures

### 3. show_alloc_mem_ex() ✅

Affiche des informations détaillées :

```
=== Debug Configuration ===
MALLOC_SCRIBBLE: ON
MALLOC_PRE_SCRIBBLE: OFF
MALLOC_STACK_LOGGING: ON
MALLOC_CHECK_: 2

=== Allocation History ===
ALLOC 0x00007F5015FDE020 size: 8192  time: 1756409765
FREE  0x00007F5015F54058 size: 1024  time: 1756409766

=== Memory Zones ===
TINY : 0x00007F5015FE3000
Zone 0x00007F5015FE3000 (65536 bytes)
0x00007F5015FE3038 - 0x00007F5015FE3068 : 48 bytes (allocated at 1756409765)
0x00007F5015FE3038  48 65 6C 6C 6F 20 77 6F 72 6C 64 20 66 72 6F 6D  |Hello world from|
0x00007F5015FE3048  20 6D 61 6C 6C 6F 63 21 00 00 00 00 00 00 00 00  | malloc!........|
```

### 4. Défragmentation mémoire ✅

```c
void defragment_zones(void) {
    // Parcourt toutes les zones TINY et SMALL
    // Fusionne les blocs libres adjacents
    // Réduit la fragmentation
}

void malloc_defragment(void) {
    // API publique thread-safe pour la défragmentation
    pthread_mutex_lock(&g_heap.mutex);
    defragment_zones();
    pthread_mutex_unlock(&g_heap.mutex);
}
```

## Optimisations et performances

### Alignement mémoire
```c
#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
```
- **16 bytes** d'alignement pour optimiser les accès processeur
- **Vectorisation** possible (SSE/AVX)

### Réduction des appels système
- **Zones pré-allouées** : 1 mmap pour 100+ allocations
- **Réutilisation** des blocs libérés
- **Fragmentation contrôlée** par split/merge

### Thread safety avec performances
- **Mutex unique** : simple mais peut créer des contentions
- **Alternative possible** : mutex per-zone (non implémenté)

## Limitations et améliorations possibles

### Limitations actuelles
1. **Coalescing unidirectionnel** : fusion seulement avec le bloc suivant
2. **Mutex global** : limite les performances multi-thread
3. **Pas de munmap automatique** des zones vides
4. **Historique limité** : 1000 entrées maximum

### Améliorations futures
1. **Coalescing bidirectionnel** avec pointeurs prev
2. **Mutex par zone** pour parallélisme
3. **Garbage collection** des zones vides  
4. **Arena allocator** pour gros programmes
5. **Red zones** complètes avec guard pages

## Tests et validation

### Tests unitaires disponibles
```bash
make test              # Tests de base
make test-debug        # Avec sanitizers
make test-valgrind     # Avec Valgrind
./demo_bonus.sh        # Démo des bonus
```

### Tests de correction
```bash
export HOSTTYPE=Testing
make re
./run_linux.sh ./test0
./run_linux.sh ./test1
```

## Utilisation

### Compilation
```bash
make                   # Compile libft_malloc.so
make symlink          # Crée le symlink
```

### Utilisation avec LD_PRELOAD
```bash
export LD_LIBRARY_PATH=.
export LD_PRELOAD=./libft_malloc.so
./your_program
```

### Ou avec le script wrapper
```bash
./run_linux.sh ./your_program
```

### Debug avec variables d'environnement
```bash
export MALLOC_SCRIBBLE=1
export MALLOC_STACK_LOGGING=1  
export MALLOC_CHECK_=2
./run_linux.sh ./your_program
```

## Conformité avec le sujet

### Partie obligatoire ✅
- [x] malloc/free/realloc implémentés
- [x] Utilisation de mmap/munmap uniquement
- [x] Zones TINY/SMALL/LARGE
- [x] Au moins 100 allocations par zone
- [x] show_alloc_mem() fonctionnelle
- [x] Gestion des erreurs robuste
- [x] Alignement correct

### Bonus ✅
- [x] Thread safety (pthread_mutex)
- [x] Variables d'environnement de debug
- [x] show_alloc_mem_ex() avec détails
- [x] Défragmentation mémoire

**Score attendu : 125/100** 🎉