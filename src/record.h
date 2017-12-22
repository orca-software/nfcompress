/**
 * \file record.h
 * \brief nfdump record parsing and handling functions
 *
 * \author J.R.Versteegh <j.r.versteegh@orca-st.com>
 *
 * \copyright
 * (C) 2017 Jaap Versteegh. All rights reserved.
 * (C) 2017 SURFnet. All rights reserved.
 * \license
 * This software may be modified and distributed under the
 * terms of the BSD license. See the LICENSE file for details.
 */

#ifndef _RECORD_H
#define _RECORD_H

#include "types.h"

#include "record.h"

typedef struct {
  union {
    L_record_header_t header;
    struct {
      uint32_t type;
      uint32_t size;
    };
  };
  char data[];
} nf_record_t;
typedef nf_record_t* nf_record_p;

extern nf_record_p record_new(const size_t size);
extern nf_record_p record_copy(const nf_record_p record);
extern void record_free(nf_record_p *record);

#endif
