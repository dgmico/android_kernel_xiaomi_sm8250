#ifndef _XT_CONNMARK_H_target
#define _XT_CONNMARK_H_target
#include <linux/types.h>
enum {
	XT_CONNMARK_SET = 0,
	XT_CONNMARK_SAVE,
	XT_CONNMARK_RESTORE,
};
struct xt_connmark_tginfo2 {
	__u32 ctmark, ctmask, nfmask;
	__u8 shift_dir, shift_bits, mode;
};
#endif
