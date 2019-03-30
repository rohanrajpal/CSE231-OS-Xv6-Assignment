// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.
//
// The implementation uses two state flags internally:
// * B_VALID: the buffer data has been read from the disk.
// * B_DIRTY: the buffer data has been modified
//     and needs to be written to disk.

#include <libzvbi.h>
#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

//PAGEBREAK!
  // Create linked list of buffers

  //bcache is a cyclic list
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    initsleeplock(&b->lock, "buffer");
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
    struct buf *b;
//    cprintf("attempting to acquire from bget blk : %d\n", blockno);
  acquire(&bcache.lock);
    //cprintf("acquired from bget blk : %d ", blockno);

//    if(blockno==694){
//        cprintf("reached 694 part1\n");
//    }
  // Is the block already cached?
  for(b = bcache.head.next; b != &bcache.head; b = b->next){
//      if(blockno==694){
//          cprintf("reached 694 part1.5 start 1.5if\n");
//      }
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
//      if(blockno==694){
//          cprintf("reached 694 part2\n");
//      }
      release(&bcache.lock);
      //cprintf("released from bget blk : %d\n", blockno);
      acquiresleep(&b->lock);
      return b;
    }
  }
//    if(blockno==694){
//        cprintf("reached 694 part3\n");
//    }

  // Not cached; recycle an unused buffer.
  // Even if refcnt==0, B_DIRTY indicates a buffer is in use
  // because log.c has modified it but not yet committed it.
//  volatile struct buf *altb = b;
//    if(altb);
  for(b = bcache.head.prev; b != &bcache.head; b = b->prev){

//      if(blockno==694){
//          cprintf("reached 694 part3.5 startif\n");
//
//          cprintf("%d %d \n",b->refcnt,b->flags);
//          cprintf("printed details\n");
//      }

    if(b->refcnt == 0 && (b->flags & B_DIRTY) == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->flags = 0;
      b->refcnt = 1;
//        if(blockno==694){
//            cprintf("reached 694 part4\n");
//        }
      release(&bcache.lock);
//        cprintf("2released from bget blk : %d\n", blockno);

        acquiresleep(&b->lock);
//        if(blockno==694){
//            volatile struct buf *balt = &bcache.head;
//            if (balt != NULL){
//                cprintf("bcachce details %d \n",balt->blockno);
//            }
//            else{
//                cprintf("danger in bcache\n");
//            }
//        }
      return b;
    }
//      if(blockno==694){
//          cprintf("reached 694 part3.5 endif\n");
//      }
  }

//  if(blockno==694){
//      cprintf("reached 694 part5\n");
//  }
  panic("bget: no buffers");
}

/* Write 4096 bytes pg to the eight consecutive
 * starting at blk.
 */
void
write_page_to_disk(uint dev, char *pg, uint blk)
{
    for(int i = 0; i < 8; i++){
        struct buf* b = bget(dev, blk + i);
//        struct buf* b_copy = b;
//        if(!holdingsleep(&b->lock))
//        {
//          panic("write page to disk before memmove");
//        }
        memmove(b->data, pg + i* BSIZE, BSIZE);

//        if(!holdingsleep(&b->lock))
//        {
//          panic("write page to disk after memmove");
//        }

        bwrite(b);
        brelse(b);
    }
}

/* Read 4096 bytes from the eight consecutive
 * starting at blk into pg.
 */
void
read_page_from_disk(uint dev, char *pg, uint blk)
{
    for(int i = 0; i < 8; i++){
        struct buf* b = bread(dev, blk + i);
        memmove(pg + i* BSIZE, b->data, BSIZE);
//        bwrite(b);
        brelse(b);
    }
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if((b->flags & B_VALID) == 0) {
    iderw(b);
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  b->flags |= B_DIRTY;
  iderw(b);
}

// Release a locked buffer.
// Move to the head of the MRU list.
//most recently used list = MRU
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");
	
  releasesleep(&b->lock);

  acquire(&bcache.lock);
  //cprintf("acquired from brelse blk: %d ", b->blockno);

  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
  
  release(&bcache.lock);
  //cprintf("released from brelse blk : %d\n", b->blockno);
}
//PAGEBREAK!
// Blank page.

