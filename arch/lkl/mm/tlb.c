#include <asm/tlbflush.h>

void flush_tlb_all(void)
{
}

void flush_tlb_mm(struct mm_struct *mm)
{
}

void flush_tlb_range(struct vm_area_struct *vma, unsigned long start,
			    unsigned long end)
{
}

void flush_tlb_page(struct vm_area_struct *vma, unsigned long address)
{
}

void flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
}