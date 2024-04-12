#ifndef _ASM_LKL_MEM_H
#define _ASM_LKL_MEM_H

extern unsigned long memory_start;

static inline unsigned long lkl_to_phys(void *virt)
{
	return(((unsigned long) virt - memory_start));
}

static inline void *lkl_to_virt(unsigned long phys)
{
	return((void *)memory_start + phys);
}

#endif // _ASM_LKL_MEM_H
