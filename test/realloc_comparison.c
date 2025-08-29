#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>

#define INITIAL_SIZE (1024 * 1024)  // 1MB
#define FINAL_SIZE   (4 * 1024 * 1024)  // 4MB
#define TEST_DATA_SIZE 256

// Structure pour simuler un bloc LARGE
typedef struct {
    size_t size;
    char data[];
} test_block_t;

// Méthode traditionnelle : mmap/munmap avec copie
void* traditional_realloc(void* old_ptr, size_t old_size, size_t new_size)
{
    if (!old_ptr)
        return mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (new_size == 0) {
        munmap(old_ptr, old_size);
        return NULL;
    }
    
    // Allouer nouvelle zone
    void* new_ptr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_ptr == MAP_FAILED)
        return NULL;
    
    // Copier les données
    size_t copy_size = (old_size < new_size) ? old_size : new_size;
    memcpy(new_ptr, old_ptr, copy_size);
    
    // Libérer l'ancienne zone
    munmap(old_ptr, old_size);
    
    return new_ptr;
}

// Méthode optimisée : mremap
void* optimized_realloc(void* old_ptr, size_t old_size, size_t new_size)
{
    if (!old_ptr)
        return mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (new_size == 0) {
        munmap(old_ptr, old_size);
        return NULL;
    }
    
    // Utiliser mremap pour expansion/réduction in-place
    void* new_ptr = mremap(old_ptr, old_size, new_size, MREMAP_MAYMOVE);
    if (new_ptr == MAP_FAILED)
        return NULL;
    
    return new_ptr;
}

// Mesure des fautes de page
long get_page_faults(void)
{
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
        return usage.ru_majflt + usage.ru_minflt;
    return -1;
}

// Test de performance d'une méthode
void test_method(const char* method_name, 
                 void* (*realloc_func)(void*, size_t, size_t))
{
    printf("\n=== Test %s ===\n", method_name);
    
    // Mesures initiales
    struct timespec start, end;
    long initial_faults = get_page_faults();
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Allocation initiale
    void* ptr = realloc_func(NULL, 0, INITIAL_SIZE);
    if (!ptr) {
        printf("Erreur: allocation initiale échouée\n");
        return;
    }
    
    // Remplir avec des données de test
    memset(ptr, 0xAA, TEST_DATA_SIZE);
    char test_pattern[] = "REALLOC_TEST_DATA_PATTERN_123456789";
    memcpy(ptr, test_pattern, strlen(test_pattern));
    
    printf("Allocation initiale: %zu bytes\n", INITIAL_SIZE);
    
    // Série d'expansions
    size_t current_size = INITIAL_SIZE;
    for (int i = 0; i < 3; i++) {
        size_t new_size = current_size * 2;
        
        ptr = realloc_func(ptr, current_size, new_size);
        if (!ptr) {
            printf("Erreur: expansion %d échouée\n", i + 1);
            return;
        }
        
        // Vérifier que les données sont préservées
        if (memcmp(ptr, test_pattern, strlen(test_pattern)) != 0) {
            printf("Erreur: données corrompues après expansion %d\n", i + 1);
            return;
        }
        
        printf("Expansion %d: %zu -> %zu bytes\n", i + 1, current_size, new_size);
        current_size = new_size;
    }
    
    // Mesures finales
    clock_gettime(CLOCK_MONOTONIC, &end);
    long final_faults = get_page_faults();
    
    // Libération
    munmap(ptr, current_size);
    
    // Calculs de performance
    double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                     (end.tv_nsec - start.tv_nsec) / 1000000.0;
    long page_faults = final_faults - initial_faults;
    
    printf("Temps d'exécution: %.2f ms\n", time_ms);
    printf("Fautes de page: %ld\n", page_faults);
    printf("Données préservées: OK\n");
}

int main(void)
{
    printf("=== Comparaison des implémentations de realloc ===\n");
    printf("Test de réallocation de %d KB à %d KB avec préservation de données\n", 
           INITIAL_SIZE / 1024, FINAL_SIZE / 1024);
    
    // Test méthode traditionnelle
    test_method("Méthode traditionnelle (mmap/munmap + memcpy)", traditional_realloc);
    
    // Test méthode optimisée
    test_method("Méthode optimisée (mremap)", optimized_realloc);
    
    printf("\n=== Analyse ===\n");
    printf("La méthode traditionnelle:\n");
    printf("  - Alloue une nouvelle zone avec mmap()\n");
    printf("  - Copie toutes les données avec memcpy()\n");
    printf("  - Libère l'ancienne zone avec munmap()\n");
    printf("  - Plus de fautes de page (nouvelles pages + copie)\n\n");
    
    printf("La méthode optimisée:\n");
    printf("  - Étend la zone existante avec mremap()\n");
    printf("  - Préserve les données automatiquement\n");
    printf("  - Évite la copie des données (MREMAP_MAYMOVE)\n");
    printf("  - Moins de fautes de page (réutilise pages existantes)\n\n");
    
    return 0;
}