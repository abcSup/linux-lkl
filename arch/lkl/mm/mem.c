#include <linux/pgtable.h>

#include <asm/pgalloc.h>

pgd_t swapper_pg_dir[PTRS_PER_PGD];

pgd_t *pgd_alloc(struct mm_struct *)
{
	pgd_t *pgd = (pgd_t *)__get_free_page(GFP_KERNEL);

	if (pgd) {
		memset(pgd, 0, USER_PTRS_PER_PGD * sizeof(pgd_t));
		memcpy(pgd + USER_PTRS_PER_PGD,
		       swapper_pg_dir + USER_PTRS_PER_PGD,
		       (PTRS_PER_PGD - USER_PTRS_PER_PGD) * sizeof(pgd_t));
	}
	return pgd;
}

static const pgprot_t protection_map[16] = {
	[VM_NONE]					= PAGE_NONE,
	[VM_READ]					= PAGE_READONLY,
	[VM_WRITE]					= PAGE_COPY,
	[VM_WRITE | VM_READ]				= PAGE_COPY,
	[VM_EXEC]					= PAGE_READONLY,
	[VM_EXEC | VM_READ]				= PAGE_READONLY,
	[VM_EXEC | VM_WRITE]				= PAGE_COPY,
	[VM_EXEC | VM_WRITE | VM_READ]			= PAGE_COPY,
	[VM_SHARED]					= PAGE_NONE,
	[VM_SHARED | VM_READ]				= PAGE_READONLY,
	[VM_SHARED | VM_WRITE]				= PAGE_SHARED,
	[VM_SHARED | VM_WRITE | VM_READ]		= PAGE_SHARED,
	[VM_SHARED | VM_EXEC]				= PAGE_READONLY,
	[VM_SHARED | VM_EXEC | VM_READ]			= PAGE_READONLY,
	[VM_SHARED | VM_EXEC | VM_WRITE]		= PAGE_SHARED,
	[VM_SHARED | VM_EXEC | VM_WRITE | VM_READ]	= PAGE_SHARED
};
DECLARE_VM_GET_PAGE_PROT
