
#include <stdlib.h>
#include "errcodes.h"


int check_buffer_size(char **buf, int *buf_size_bytes, int requested_size_bytes)
{
  int ret = APP_ERR_NONE;

  if(*buf_size_bytes >= requested_size_bytes)
    {
      return APP_ERR_NONE;
    }

  free(*buf);
  *buf = (char *) malloc(requested_size_bytes);
  if(*buf != NULL) {
    *buf_size_bytes = requested_size_bytes;
  }
  else {
    *buf_size_bytes = 0;
    ret = APP_ERR_MALLOC;
  }

  return ret;
}
