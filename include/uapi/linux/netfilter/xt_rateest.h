/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _XT_RATEEST_TARGET_H
#define _XT_RATEEST_TARGET_H

#include <linux/types.h>
#include <linux/if.h>

struct xt_rateest_target_info {
	char			name[IFNAMSIZ];
	__s8			interval;
	__u8		ewma_log;

	/* Used internally by the kernel */
	struct xt_rateest	*est __attribute__((aligned(8)));
};

#endif /* _XT_RATEEST_TARGET_H */
#ifndef _XT_RATEEST_H
#define _XT_RATEEST_H
#include <linux/types.h>
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
struct xt_rateest;
struct xt_rateest_target_info {
	char		name[IFNAMSIZ];
	__u8		interval;
	__u8		ewma_log;
	struct xt_rateest	*est __attribute__((aligned(8)));
};
struct xt_rateest_match_info {
	char		name1[IFNAMSIZ];
	char		name2[IFNAMSIZ];
	__u16		flags;
	__u16		mode;
	__u32		bps1, pps1;
	__u32		bps2, pps2;
	struct xt_rateest *est1 __attribute__((aligned(8)));
	struct xt_rateest *est2 __attribute__((aligned(8)));
};
#endif
