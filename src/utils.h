/** 
 * \file utils.h
 * \brief Generic tool utilities
 *
 * \author J.R.Versteegh <j.r.versteegh@orca-st.com>
 *
 * \copyright (C) 2017 SURFnet. All rights reserved.
 * \license This software may be modified and distributed under the 
 * terms of the BSD license. See the LICENSE file for details.
 */

#ifndef _UTILS_H
#define _UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { log_debug, log_info, log_error } log_level_t;

extern void msg(log_level_t log_level, const char *message, ...);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
