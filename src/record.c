/** 
 * \file record.c
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

#include <stdlib.h>
#include <string.h>

#include "block.h"

nf_record_p record_new(const size_t size)
{ 
  return (nf_record_p)calloc(1, sizeof(nf_record_t));
}

nf_record_p record_copy(const nf_record_p record)
{
  nf_record_p result = (nf_record_p)malloc(record->size + sizeof(nf_record_t));
  *result = *record;
  memcpy(result->data, record->data, record->size);
  return result;
}

void record_free(nf_record_p *record) {
  if (*record == NULL)
    return;
  nf_record_p rc = *record;
  *record = NULL;
  free(rc);
}
