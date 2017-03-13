#ifdef BARE_METAL

#include <stdio.h>
int benchmark();

#define RUNS 1

int main(void)
{
  int run;

  // Run baseline
  //puts("Baseline\n\r");
  //benchmark();
  for(run = 0; run < RUNS; ++run)
  {
    printf("Run %d\n\r", run+1);
    benchmark();
  }

  return 0;
}

//#define printf(...)
//#define fprintf(...)
#define main(...) benchmark(__VA_ARGS__)

#endif
