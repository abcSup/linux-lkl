#ifndef _ASM_LKL_PGALLOC_H
#define _ASM_LKL_PGALLOC_H

#include <linux/mm.h>

#include <asm-generic/pgalloc.h>

#define pmd_populate_kernel(mm, pmd, pte) \
	set_pmd(pmd, __pmd(_PAGE_TABLE + (unsigned long) __pa(pte)))

#define pmd_populate(mm, pmd, pte) 				\
	set_pmd(pmd, __pmd(_PAGE_TABLE +			\
		((unsigned long long)page_to_pfn(pte) <<	\
			(unsigned long long) PAGE_SHIFT)))

/*
 * Allocate and free page tables.
 */
extern pgd_t *pgd_alloc(struct mm_struct *);

#define __pte_free_tlb(tlb,pte, address)		\
do {							\
	pgtable_pte_page_dtor(pte);			\
	tlb_remove_page((tlb),(pte));			\
} while (0)

#endif /* _ASM_LKL_PGALLOC_H */