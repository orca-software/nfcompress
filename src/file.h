#ifndef _FILE_H
#define _FILE_H

extern nf_file_t* load(const char* filename, block_handler_p handle_block);
extern int save(const char* filename, nf_file_t* fl);
extern void for_each_block(nf_file_t* fl, block_handler_p handle_block);
extern void free_file(nf_file_t* fl);

#endif
