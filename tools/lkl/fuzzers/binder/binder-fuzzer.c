// SPDX-License-Identifier: GPL-2.0
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <lkl.h>
#include <lkl_host.h>

#include "binder.h"

#define LKL_CALL(op)	lkl_sys_##op

#define LOG(fmt, ...) \
	do { \
		if (g_log_enabled) {\
			printf(fmt, ##__VA_ARGS__); \
		} \
	} while (0)

bool g_log_enabled = true;

static int initialize_lkl(void)
{
	if (!g_log_enabled)
		lkl_host_ops.print = NULL;

	int ret = lkl_init(&lkl_host_ops);

	if (ret) {
		LOG("lkl_init failed\n");
		return -1;
	}

	ret = lkl_start_kernel("mem=50M kasan.fault=panic");
	if (ret) {
		LOG("lkl_start_kernel failed\n");
		lkl_cleanup();
		return -1;
	}

	lkl_mount_fs("sysfs");
	lkl_mount_fs("proc");

	ret = lkl_sys_mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);
	if (ret < 0 || ret != 0) {
		LOG("lkl_sys_mount failed\n");
		lkl_cleanup();
		return -1;
	}

	// Test binder is loaded
	binder_ctx *ctx = binder_open();
	if (!ctx) {
		LOG("binder_open failed\n");
		lkl_cleanup();
		return -1;
	}
	ret = binder_ioctl_check_version(ctx);
	if (ret) {
		LOG("binder_ioctl_check_version failed\n");
		lkl_cleanup();
		return -1;
	}
	binder_close(ctx);

	return 0;
}

// extern int __llvm_profile_write_file(void) __attribute__((weak));
// extern int __llvm_profile_initialize_file(void) __attribute__((weak));

void flush_coverage(void)
{
	LOG("Flushing coverage data...\n");
	// __llvm_profile_write_file();
	LOG("Done...\n");
}

int LLVMFuzzerInitialize(int *argc, char ***argv)
{
	initialize_lkl();

	// __llvm_profile_initialize_file();
	atexit(flush_coverage);

	return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
	binder_ctx *ctx = binder_open();
	if (!ctx) {
		LOG("binder_open failed\n");
		lkl_cleanup();
		return -1;
	}

	binder_close(ctx);

	return 0;
}

