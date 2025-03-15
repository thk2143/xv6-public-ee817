#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

extern char data[];  // defined by kernel.ld

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;
  struct proc *curproc = myproc();
  oldpgdir = curproc->pgdir;
  // kernal mapping. vm.c에서 복사한 코드
  static struct kmap {
    void *virt;
    uint phys_start;
    uint phys_end;
    int perm;
  } kmap[] = {
   { (void*)KERNBASE, 0,             EXTMEM,    PTE_W}, // I/O space
   { (void*)KERNLINK, V2P(KERNLINK), V2P(data), 0},     // kern text+rodata
   { (void*)data,     V2P(data),     PHYSTOP,   PTE_W}, // kern data+memory
   { (void*)DEVSPACE, DEVSPACE,      0,         PTE_W}, // more devices
  };
  struct kmap *k;


  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  // if((pgdir = setupkvm()) == 0)
  //   goto bad;
  
  // kalloc으로 4096-byte page를 할당받아 pgdir에 저장
  if((pgdir = (pde_t*)kalloc()) == 0)
    return 0;
  
  // pgdir의 모든 entry를 0으로 초기화
  memset(pgdir, 0, PGSIZE);
  if (P2V(PHYSTOP) > (void*)DEVSPACE)
    panic("PHYSTOP too high");
  
  // oldpgdir에 저장된 kernal mapping 정보를 pgdir에 매핑
  for(k=kmap; k<&kmap[NELEM(kmap)]; k++){
    char *a = (char *)PGROUNDDOWN((uint)k->virt);
    char *end = (char *)PGROUNDDOWN((uint)k->virt + (k->phys_end - k->phys_start) - 1);
    pde_t *pde;

    for (;;){
      pde = &oldpgdir[PDX(a)];
      // pde가 존재하지 않거나, pde가 present가 아닌 경우 bad로 이동
      if (!(*pde & PTE_P)){
        goto bad;
      }
      pgdir[PDX(a)] = *pde;

      if (a == end)
        break;
      a += PGSIZE;
    }
  }


  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));

  // Commit to the user image.
  // oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;
  curproc->sz = sz;
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  switchuvm(curproc);
  freevm(oldpgdir);
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
