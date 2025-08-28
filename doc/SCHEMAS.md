# Schémas et diagrammes - Custom Malloc

## 1. Vue d'ensemble de l'architecture

```
                            ┌─────────────────────────────┐
                            │         g_heap              │
                            │   (Global Heap Manager)    │
                            └─────────────┬───────────────┘
                                          │
                    ┌─────────────────────┼─────────────────────┐
                    │                     │                     │
                    ▼                     ▼                     ▼
          ┌─────────────────┐   ┌─────────────────┐   ┌─────────────────┐
          │   TINY zones    │   │   SMALL zones   │   │  LARGE blocks   │
          │   (≤ 512B)      │   │   (≤ 4096B)     │   │   (> 4096B)     │
          └─────────────────┘   └─────────────────┘   └─────────────────┘
                    │                     │                     │
          ┌─────────▼─────────┐ ┌─────────▼─────────┐ ┌─────────▼─────────┐
          │ Zone: 64KB        │ │ Zone: 512KB       │ │ Individual mmap   │
          │ Multiple blocks   │ │ Multiple blocks   │ │ One block each    │
          └───────────────────┘ └───────────────────┘ └───────────────────┘
```

## 2. Structure détaillée du g_heap

```
g_heap (Structure globale)
┌─────────────────────────────────────────────────────────────────────────┐
│                                t_heap                                   │
├─────────────────────────────────────────────────────────────────────────┤
│ t_zone *tiny         │ Pointeur vers première zone TINY               │
│ t_zone *small        │ Pointeur vers première zone SMALL              │
│ t_block *large       │ Pointeur vers premier bloc LARGE               │
│ pthread_mutex_t mutex│ Mutex pour thread safety                       │
│ t_debug_flags debug  │ Flags de débogage (BONUS)                      │
│ t_alloc_history *hist│ Historique des allocations (BONUS)             │
│ size_t history_count │ Nombre d'entrées dans l'historique             │
└─────────────────────────────────────────────────────────────────────────┘
                    │              │              │
                    ▼              ▼              ▼
              [Zones TINY]   [Zones SMALL]   [Blocs LARGE]
```

## 3. Organisation des zones TINY/SMALL

```
Zone TINY (64KB via mmap)
┌─────────────────────────────────────────────────────────────────────────┐
│                              t_zone                                     │
├─────────────────┬───────────────┬───────────────────────────────────────┤
│ size_t size     │ t_zone *next  │ t_block *blocks                       │
│ 65536           │ ptr_next_zone │ ptr_first_block                       │
└─────────────────┴───────────────┴───────────────────────────────────────┘
│
├── Block #1 (Occupé) ──────────────────────────────────────────────────────
│   ┌─────────────────────────────────────────────────────────────────────┐
│   │                          t_block                                    │
│   ├─────────┬──────────┬─────────────┬─────────────────────────────────┤
│   │ size    │ is_free  │ t_block*next│ time_t alloc_time               │
│   │ 96      │ false    │ ptr_block2  │ 1756409765                      │
│   └─────────┴──────────┴─────────────┴─────────────────────────────────┘
│   ┌─────────────────────────────────────────────────────────────────────┐
│   │                     USER DATA (96 bytes)                           │
│   │ "Hello World"                                                       │
│   │ 48 65 6C 6C 6F 20 57 6F 72 6C 64 00 00 ...                        │
│   └─────────────────────────────────────────────────────────────────────┘
│
├── Block #2 (Libre) ──────────────────────────────────────────────────────
│   ┌─────────────────────────────────────────────────────────────────────┐
│   │                          t_block                                    │
│   ├─────────┬──────────┬─────────────┬─────────────────────────────────┤
│   │ size    │ is_free  │ t_block*next│ time_t alloc_time               │
│   │ 128     │ true     │ ptr_block3  │ 0                               │
│   └─────────┴──────────┴─────────────┴─────────────────────────────────┘
│   ┌─────────────────────────────────────────────────────────────────────┐
│   │                    FREE SPACE (128 bytes)                          │
│   │ [Peut être scribbled avec 0xDE si MALLOC_SCRIBBLE=1]               │
│   └─────────────────────────────────────────────────────────────────────┘
│
├── Block #3 (Occupé) ──────────────────────────────────────────────────────
│   ...
│
└── Reste de la zone (libre) ──────────────────────────────────────────────
    ┌─────────────────────────────────────────────────────────────────────┐
    │                    AVAILABLE SPACE                                  │
    │                 (peut contenir 100+ blocks)                        │
    └─────────────────────────────────────────────────────────────────────┘
```

## 4. Organisation des blocs LARGE

```
Bloc LARGE (mmap individuel pour chaque allocation > 4096 bytes)
┌─────────────────────────────────────────────────────────────────────────┐
│                              t_block                                    │
├─────────┬──────────┬─────────────┬─────────────────────────────────────┤
│ size    │ is_free  │ t_block*next│ time_t alloc_time                   │
│ 8192    │ false    │ ptr_next    │ 1756409765                          │
└─────────┴──────────┴─────────────┴─────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────────────────┐
│                                                                         │
│                        USER DATA (8192 bytes)                          │
│                                                                         │
│ "This is a large allocation for testing purposes..."                   │
│ 54 68 69 73 20 69 73 20 61 20 6C 61 72 67 65 ...                     │
│                                                                         │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘

Note: Chaque bloc LARGE est un mmap() séparé
      Libéré avec munmap() dans free()
```

## 5. Chaînage des zones et blocs

```
g_heap.tiny ──► Zone1 ──► Zone2 ──► Zone3 ──► NULL
                 │         │         │
                 ▼         ▼         ▼
              Block1    Block1    Block1
                 │         │         │
                 ▼         ▼         ▼
              Block2    Block2    Block2
                 │         │         │
                 ▼         ▼         ▼
               NULL      NULL      NULL

g_heap.small ──► Zone1 ──► Zone2 ──► NULL
                  │         │
                  ▼         ▼
               Block1    Block1
                  │         │
                  ▼         ▼
               Block2    Block2
                  │         │
                  ▼         ▼
                NULL      NULL

g_heap.large ──► Block1 ──► Block2 ──► Block3 ──► NULL
                (mmap1)    (mmap2)    (mmap3)
```

## 6. Algorithme de recherche et allocation

```
malloc(size) {
    size = ALIGN(size, 16)
    
    if (size ≤ 512) {
        ┌─────────────────┐
        │ Chercher dans   │
        │ zones TINY      │
        └─────────────────┘
                │
                ▼
        ┌─────────────────┐      ┌─────────────────┐
        │ Zone existe ?   │ NON  │ Créer nouvelle  │
        │                 ├─────►│ zone TINY       │
        └─────────────────┘      └─────────────────┘
                │ OUI                     │
                ▼                        │
        ┌─────────────────┐               │
        │ Bloc libre      │               │
        │ suffisant ?     │               │
        └─────────────────┘               │
                │ OUI            NON      │
                ▼                │        │
        ┌─────────────────┐      │        │
        │ Split si trop   │      │        │
        │ grand           │      │        │
        └─────────────────┘      │        │
                │                │        │
                ▼                ▼        ▼
        ┌─────────────────┐ ┌─────────────────┐
        │ Retourner       │ │ Allouer dans    │
        │ pointeur        │ │ nouvelle zone   │
        └─────────────────┘ └─────────────────┘
    }
    
    else if (size ≤ 4096) {
        [Même logique pour zones SMALL]
    }
    
    else {
        ┌─────────────────┐
        │ mmap() direct   │
        │ pour bloc LARGE │
        └─────────────────┘
    }
}
```

## 7. Mécanisme de fusion (défragmentation)

```
AVANT fusion:
Block1(LIBRE) ──► Block2(LIBRE) ──► Block3(OCCUPÉ) ──► NULL
[size: 64]        [size: 96]        [size: 128]

APRÈS fusion:
Block1(LIBRE) ──────────────────────► Block3(OCCUPÉ) ──► NULL  
[size: 64+96+sizeof(t_block)]         [size: 128]

Code simplifié:
if (current->is_free && current->next->is_free) {
    current->size += current->next->size + sizeof(t_block);
    current->next = current->next->next;
}
```

## 8. Structures bonus - Debug et historique

```
t_debug_flags (Flags de débogage)
┌─────────────────────────────────────────────────────────────┐
│ bool scribble        │ MALLOC_SCRIBBLE (0xDE sur free)     │
│ bool pre_scribble    │ MALLOC_PRE_SCRIBBLE (0xAA sur alloc)│
│ bool guard           │ MALLOC_GUARD (protection)           │
│ bool stack_logging   │ MALLOC_STACK_LOGGING (historique)   │
│ int check_level      │ MALLOC_CHECK_ (0-3)                 │
└─────────────────────────────────────────────────────────────┘

t_alloc_history (Historique des allocations)
┌─────────────────────────────────────────────────────────────┐
│ Entry1 ──► Entry2 ──► Entry3 ──► ... ──► NULL              │
│                                                             │
│ Chaque entrée:                                              │
│ ┌─────────────────────────────────────────────────────────┐ │
│ │ void *ptr        │ Adresse allouée                     │ │
│ │ size_t size      │ Taille                              │ │
│ │ time_t timestamp │ Moment de l'allocation              │ │
│ │ bool is_freed    │ true si libéré, false si alloué    │ │
│ │ next             │ Pointeur vers entrée suivante      │ │
│ └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## 9. Flux d'exécution avec thread safety

```
Thread 1                    Thread 2                    Thread 3
    │                           │                           │
    ▼                           ▼                           ▼
malloc(100)                 free(ptr1)                 realloc(ptr2, 200)
    │                           │                           │
    ▼                           ▼                           ▼
pthread_mutex_lock()        pthread_mutex_lock()        pthread_mutex_lock()
    │                           │                           │
    ▼                           ▼                           ▼
[WAIT]  ◄───────────────── [CRITICAL SECTION]           [WAIT]
    │                           │                           │
    ▼                           ▼                           ▼
[CRITICAL SECTION] ────────► pthread_mutex_unlock()      [WAIT]
    │                           │                           │
    ▼                           ▼                           ▼
pthread_mutex_unlock()      [DONE]                  [CRITICAL SECTION]
    │                                                       │
    ▼                                                       ▼
[DONE]                                            pthread_mutex_unlock()
                                                            │
                                                            ▼
                                                         [DONE]

Note: Un seul thread peut accéder au heap à la fois
      Garantit la cohérence mais limite les performances
```

## 10. Exemple concret d'allocation

```
Séquence: malloc(50) + malloc(1000) + malloc(5000)

État initial:
g_heap.tiny = NULL
g_heap.small = NULL  
g_heap.large = NULL

Après malloc(50):
g_heap.tiny ──► [Zone TINY 64KB]
                      │
                      ▼
                 [Block: 50B utilisé] ──► [Block: ~64KB libre]

Après malloc(1000):  
g_heap.tiny ──► [Zone TINY 64KB]
g_heap.small ──► [Zone SMALL 512KB]
                       │
                       ▼
                  [Block: 1000B utilisé] ──► [Block: ~512KB libre]

Après malloc(5000):
g_heap.tiny ──► [Zone TINY 64KB]
g_heap.small ──► [Zone SMALL 512KB]  
g_heap.large ──► [Block LARGE 5000B] ──► NULL
                 (mmap séparé)
```

## 11. Optimisations mémoire

```
Alignement 16 bytes:
Original size: 13    ───► Aligned size: 16
Original size: 17    ───► Aligned size: 32
Original size: 33    ───► Aligned size: 48

Macro ALIGN:
#define ALIGN(size) (((size) + 15) & ~15)

Avantages:
- Optimisation processeur (accès alignés)
- Compatible vectorisation (SSE/AVX)
- Évite les pénalités de performance
```

Cette documentation avec schémas détaillés permet de comprendre parfaitement l'architecture et le fonctionnement interne de l'implémentation malloc ! 🎯