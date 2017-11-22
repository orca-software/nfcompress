#ifndef _UTILS_H
#define _UTILS_H

typedef enum { log_debug, log_info, log_error } log_level_t;

extern void msg(log_level_t log_level, const char *message, ...);

#endif
