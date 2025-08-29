#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

long get_memory_usage() {
    FILE *status = fopen("/proc/self/status", "r");
    if (!status) return -1;
    
    char line[256];
    long vmrss = 0;
    
    while (fgets(line, sizeof(line), status)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %ld kB", &vmrss);
            break;
        }
    }
    fclose(status);
    return vmrss * 1024; // Convert to bytes
}

void test_allocation_pattern() {
    long initial_memory = get_memory_usage();
    void *ptrs[1000];
    
    // Allocate various sizes to stress different zones
    for (int i = 0; i < 1000; i++) {
        if (i < 400) {
            ptrs[i] = malloc(64);  // TINY zone
        } else if (i < 800) {
            ptrs[i] = malloc(1024); // SMALL zone  
        } else {
            ptrs[i] = malloc(8192); // LARGE blocks
        }
        
        if (ptrs[i]) {
            memset(ptrs[i], 0x42, i < 400 ? 64 : (i < 800 ? 1024 : 8192));
        }
    }
    
    long peak_memory = get_memory_usage();
    
    // Free everything
    for (int i = 0; i < 1000; i++) {
        free(ptrs[i]);
    }
    
    long final_memory = get_memory_usage();
    
    printf("Initial: %ld bytes, Peak: %ld bytes, Final: %ld bytes\n", 
           initial_memory, peak_memory, final_memory);
    printf("Peak overhead: %ld bytes (%.2f pages)\n", 
           peak_memory - initial_memory, 
           (double)(peak_memory - initial_memory) / getpagesize());
}

int main() {
    printf("Page size: %ld bytes\n", getpagesize());
    test_allocation_pattern();
    return 0;
}