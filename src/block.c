/** 
 * \file block.c
 * \brief nfdump block parsing and handlingfunctions
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

#include "block.h"

nf_block_p block_new()
{ 
  return (nf_block_p)calloc(1, sizeof(nf_block_t));
}

void block_free(nf_block_p *block) {
  if (*block == NULL)
    return;
  nf_block_p bl = *block;
  *block = NULL;
  free(bl->records);
  free(bl->data);
  free(bl);
}
