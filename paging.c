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


/* Allocate eight consecutive disk blocks.
 * Save the content of the physical page in the pte
 * to the disk blocks and save the block-id into the
 * pte.
 */
void
swap_page_from_pte(pte_t *pte)
{
	uint b = balloc_page(1);
	uint pa = PTE_ADDR(*pte);
	//unsure
	char* v = P2V(pa);
	write_page_to_disk(1,v,b);
	//what is this
	*pte = b | PTE_SWAP;
	asm volatile(" invlpg %0 ": : "r"(v) );

//	kfree(v);

	//*pte &= !PTE_P Maybe in demand paging ( Marking as invalid )
}

/* Select a victim and swap the contents to the disk.
 */
int
swap_page(pde_t *pgdir)
{
	panic("swap_page is not implemented");
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

    *pte = V2P( kalloc() );

    char *ans = kalloc();
    cprintf(ans);
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
