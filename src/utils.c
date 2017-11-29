/** 
 * \file utils.c
 * \brief Generic tool utilities
 *
 * \author J.R.Versteegh <j.r.versteegh@orca-st.com>
 *
 * \copyright (C) 2017 SURFnet. All rights reserved.
 * \license This software may be modified and distributed under the 
 * terms of the BSD license. See the LICENSE file for details.
 */

#include <stdio.h>
#include <stdarg.h>

#include "types.h"
#include "utils.h"

void msg(log_level_t log_level, const char *message, ...)
{
  va_list args;
  va_start(args, message);
  switch (log_level) {
    case (log_debug):
#ifdef DEBUG
      #pragma omp critical
      vprintf(message, args);
#endif
      break;
    case (log_info):
      #pragma omp critical
      vprintf(message, args);
      break;
    case (log_error):
      #pragma omp critical
      vfprintf(stderr, message, args);
      break;
     default:
      #pragma omp critical
      fprintf(stderr, "Unknown loglevel\n");
  }
  va_end(args);
}

