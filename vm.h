//
// Created by rohan on 27/3/19.
//

#ifndef VM_H
#define VM_H

//#include "types.h"
//#include "mmu.h"

pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc);

#endif //XV6_PAGING_VM_H
