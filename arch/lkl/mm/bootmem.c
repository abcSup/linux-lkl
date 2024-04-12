// SPDX-License-Identifier: GPL-2.0
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/swap.h>

unsigned long memory_start, memory_end;
static unsigned long _memory_start, mem_size;

void *empty_zero_page;

#ifdef CONFIG_MMU
static void __init one_page_table_init(pmd_t *pmd)
{
	if (pmd_none(*pmd)) {
		pte_t *pte = (pte_t *) memblock_alloc_low(PAGE_SIZE,
							  PAGE_SIZE);
		if (!pte)
			panic("%s: Failed to allocate %lu bytes align=%lx\n",
			      __func__, PAGE_SIZE, PAGE_SIZE);

		set_pmd(pmd, __pmd(_KERNPG_TABLE +
					   (unsigned long) __pa(pte)));
		if (pte != pte_offset_kernel(pmd, 0))
			BUG();
	}
}

static void __init fixrange_init(unsigned long start, unsigned long end,
				 pgd_t *pgd_base)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	int i, j;
	unsigned long vaddr;

	vaddr = start;
	i = pgd_index(vaddr);
	j = pmd_index(vaddr);
	pgd = pgd_base + i;

	for ( ; (i < PTRS_PER_PGD) && (vaddr < end); pgd++, i++) {
		p4d = p4d_offset(pgd, vaddr);
		pud = pud_offset(p4d, vaddr);
		pmd = pmd_offset(pud, vaddr);
		for (; (j < PTRS_PER_PMD) && (vaddr < end); pmd++, j++) {
			one_page_table_init(pmd);
			vaddr += PMD_SIZE;
		}
		j = 0;
	}
}
#endif

void __init bootmem_init(unsigned long mem_sz)
{
	unsigned long zones_max_pfn[MAX_NR_ZONES] = {0, };

	mem_size = mem_sz;

	if (lkl_ops->page_alloc) {
		mem_size = PAGE_ALIGN(mem_size);
		_memory_start = (unsigned long)lkl_ops->page_alloc(mem_size);
	} else {
		_memory_start = (unsigned long)lkl_ops->mem_alloc(mem_size);
	}

	memory_start = _memory_start;
	BUG_ON(!memory_start);
	memory_end = memory_start + mem_size;

	if (PAGE_ALIGN(memory_start) != memory_start) {
		mem_size -= PAGE_ALIGN(memory_start) - memory_start;
		memory_start = PAGE_ALIGN(memory_start);
		mem_size = (mem_size / PAGE_SIZE) * PAGE_SIZE;
	}
	pr_info("memblock address range: 0x%lx - 0x%lx\n", memory_start,
		memory_start+mem_size);
	/*
	 * Give all the memory to the bootmap allocator, tell it to put the
	 * boot mem_map at the start of memory.
	 */
	max_low_pfn = virt_to_pfn(memory_end);
	min_low_pfn = virt_to_pfn(memory_start);
	memblock_add(__pa(memory_start), mem_size);

	empty_zero_page = memblock_alloc(PAGE_SIZE, PAGE_SIZE);
	memset((void *)empty_zero_page, 0, PAGE_SIZE);

	zones_max_pfn[ZONE_NORMAL] = max_low_pfn;
	free_area_init(zones_max_pfn);

#ifdef CONFIG_MMU
	/*
	 * Fixed mappings, only the page table structure has to be
	 * created - mappings will be set by set_fixmap():
	 */
	// TODO: `__end_of_fixed_addresses - 1` is negative
	unsigned long vaddr = __fix_to_virt(__end_of_fixed_addresses - 1) & PMD_MASK;
	fixrange_init(vaddr, FIXADDR_TOP, swapper_pg_dir);
#endif /* CONFIG_MMU */
}

void __init mem_init(void)
{
	memblock_free_all();
	max_low_pfn = totalram_pages();
	max_pfn = max_low_pfn;
	max_mapnr = max_pfn;
	pr_info("Memory available: %luk/%luk RAM\n",
		(nr_free_pages() << PAGE_SHIFT) >> 10, mem_size >> 10);
}

/*
 * In our case __init memory is not part of the page allocator so there is
 * nothing to free.
 */
void free_initmem(void)
{
}

void free_mem(void)
{
	if (lkl_ops->page_free)
		lkl_ops->page_free((void *)_memory_start, mem_size);
	else
		lkl_ops->mem_free((void *)_memory_start);
}
