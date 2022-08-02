// See LICENSE for license details.

#ifndef BARE_MALLOC_H
#define BARE_MALLOC_H

#include <stdint.h>

// align to cache size
typedef struct bareMalloc_st_ {
  uint64_t start_heap;
  uint64_t end_heap;
  uint64_t dummy[6];
} bareMalloc_t;

  
#ifdef __cplusplus
extern "C" {
#endif
  /*
   * Function Prototypes
   */
  void* bare_malloc(uint32_t nbytes);
  void bare_free(void *ptr);

// end of extern
#ifdef __cplusplus
}
#endif

#endif
