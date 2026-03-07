#ifndef _XT_RATEEST_TARGET_H
#define _XT_RATEEST_TARGET_H
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
#endif
