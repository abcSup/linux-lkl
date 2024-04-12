#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "binder.h"

#define ERR(fmt, ...) printf("ERROR: " fmt "\n", ##__VA_ARGS__)

#define BINDER_DEVICE "/dev/binder"
#define BINDER_VM_SIZE 1 * 1024 * 1024

#define PAD_SIZE_UNSAFE(s) (((s) + 3) & ~3UL)

binder_ctx *binder_open()
{
	binder_ctx *ctx;

	ctx = calloc(1, sizeof(binder_ctx));
	if (!ctx)
		return NULL;

	ctx->fd =
		lkl_sys_open(BINDER_DEVICE, O_RDWR | O_CLOEXEC | O_NONBLOCK, 0);
	if (ctx->fd == -1) {
		ERR("Failed to open binder device");
		goto err_open;
	}

	ctx->epoll_fd = -1;
	ctx->map_size = BINDER_VM_SIZE;
	ctx->map_ptr = lkl_sys_mmap((void *)0x1000, BINDER_VM_SIZE, PROT_READ,
				    MAP_PRIVATE, ctx->fd, 0);
	if (ctx->map_ptr == MAP_FAILED) {
		ERR("Failed to mmap binder device");
		goto err_mmap;
	}

	return ctx;
err_mmap:
	lkl_sys_close(ctx->fd);
err_open:
	free(ctx);
	return NULL;
}

void binder_close(binder_ctx *ctx)
{
	if (ctx) {
		lkl_sys_munmap((unsigned long)ctx->map_ptr, ctx->map_size);
		lkl_sys_close(ctx->fd);
		if (ctx->epoll_fd != -1) {
			lkl_sys_close(ctx->epoll_fd);
		}
		free(ctx);
	}
}

int binder_ioctl_set_context_manager(binder_ctx *ctx)
{
	int ret;

	ret = lkl_sys_ioctl(ctx->fd, LKL_BINDER_SET_CONTEXT_MGR, 0);
	if (ret < 0) {
		ERR("Failed to set context manager");
	}
	return ret;
}

int binder_ioctl_write(binder_ctx *ctx, void *buffer, size_t size)
{
	int ret;
	struct lkl_binder_write_read bwr = {
		.write_size = size,
		.write_consumed = 0,
		.write_buffer = (lkl_binder_uintptr_t)buffer
	};

	ret = lkl_sys_ioctl(ctx->fd, LKL_BINDER_WRITE_READ,
			    (unsigned long)&bwr);
	if (ret < 0)
		return ret;

	return bwr.write_consumed;
}

int binder_ioctl_read(binder_ctx *ctx, void *buffer, size_t size,
		      lkl_binder_size_t *read_consumed)
{
	int ret;

	struct lkl_binder_write_read bwr = {
		.read_size = size,
		.read_consumed = 0,
		.read_buffer = (lkl_binder_uintptr_t)buffer
	};

	ret = lkl_sys_ioctl(ctx->fd, LKL_BINDER_WRITE_READ,
			    (unsigned long)&bwr);
	if (ret == 0) {
		*read_consumed = bwr.read_consumed;
	}

	return ret;
}

int binder_ioctl_thread_exit(binder_ctx *ctx)
{
	int ret;

	ret = lkl_sys_ioctl(ctx->fd, LKL_BINDER_THREAD_EXIT, 0);
	if (ret < 0) {
		ERR("Failed to perform thread exit");
	}
	return ret;
}

int binder_ioctl_check_version(binder_ctx *ctx)
{
	int ret;
	struct lkl_binder_version version = { 0 };

	ret = lkl_sys_ioctl(ctx->fd, LKL_BINDER_VERSION,
			    (unsigned long)&version);
	if (ret < 0) {
		return ret;
	} else if (version.protocol_version !=
		   LKL_BINDER_CURRENT_PROTOCOL_VERSION) {
		ERR("Binder version does not match: %u",
		    version.protocol_version);
		return -1;
	}

	return 0;
}

int binder_ioctl_get_node_debug_info(binder_ctx *ctx, lkl_binder_uintptr_t ptr)
{
	struct lkl_binder_node_debug_info info = { .ptr = ptr };

	return lkl_sys_ioctl(ctx->fd, LKL_BINDER_GET_NODE_DEBUG_INFO,
			     (unsigned long)&info);
}

int binder_ioctl_get_node_info_for_ref(binder_ctx *ctx, uint32_t handle)
{
	struct lkl_binder_node_info_for_ref info = { .handle = handle };

	return lkl_sys_ioctl(ctx->fd, LKL_BINDER_GET_NODE_INFO_FOR_REF,
			     (unsigned long)&info);
}

int binder_ioctl_enable_oneway_spam_detection(binder_ctx *ctx, int e)
{
	uint32_t enable = e;

	return lkl_sys_ioctl(ctx->fd, LKL_BINDER_ENABLE_ONEWAY_SPAM_DETECTION,
			     (unsigned long)&enable);
}
