#include <stdint.h>
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "paging.h"
#include "fs.h"
#include "vm.h"
//#include "user.h"
/* Allocate eight consecutive disk blocks.
 * Save the content of the physical page in the pte
 * to the disk blocks and save the block-id into the
 * pte.
 */
void
swap_page_from_pte(pte_t *pte)
{
//    panic("reached swap_page_from_pte");
	uint b = balloc_page(1);
	uint pa = PTE_ADDR(*pte);
	//unsure
	char* v = P2V(pa);
//    unsigned v = P2V(pa);
	write_page_to_disk(1,v,b);
	//what is this
	//maximum block size is FSSIZE
	*pte = (b<<12) | PTE_SWAP;
//    unsigned long tosend =0;
	asm volatile("invlpg (%0)":: "r"( (unsigned long) v) : "memory");

	kfree(v);

	//*pte &= !PTE_P Maybe in demand paging ( Marking as invalid )
}

/* Select a victim and swap the contents to the disk.
 */
int
swap_page(pde_t *pgdir)
{
    pte_t *victim = select_a_victim(pgdir);
    begin_op();
    swap_page_from_pte(victim);
    end_op();
//    syscall();
//	panic("swap_page is not implemented");
	return 1;
}

/* Map a physical page to the virtual address addr.
 * If the page table entry points to a swapped block
 * restore the content of the page from the swapped
 * block and free the swapped block.
 */
void
map_address(pde_t *pgdir, uint addr)
{
//	return;
    /*
     * walkpgdir(pg)
     */
    pte_t *pte = walkpgdir(pgdir, (void *) addr, 1);
    int bid =-1;
    if (*pte & PTE_SWAP){
        bid = getswappedblk(pgdir,addr);
        if(bid == -1){
            panic("panic in mapaddress bid is -1");
        }
    }
    char* allocmem = kalloc();

    while(1){
        if(allocmem) {
            *pte = V2P(allocmem) | PTE_P | PTE_U | PTE_W;
            break;
        }
        swap_page(pgdir);
        allocmem=kalloc();
    }
    if (bid != -1){
        read_page_from_disk(1,allocmem,bid);
        bfree_page(1,bid);
    }
//    *pte = V2P( kalloc() ) | PTE_P | PTE_U | PTE_W;
//	select_a_victim(pgdir);

//    char *ans = kalloc();
//    cprintf(ans);
//	panic("map_address is not implemented");
}

/* page fault handler */
void
handle_pgfault()
{
	unsigned addr;
	struct proc *curproc = myproc();

	asm volatile ("movl %%cr2, %0 \n\t" : "=r" (addr));
	addr &= ~0xfff;
	map_address(curproc->pgdir, addr);
}
