/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _XT_MARK_UAPI_H
#define _XT_MARK_UAPI_H

#include <linux/types.h>

/* From xt_MARK.h (Target) */
struct xt_mark_tginfo2 {
       __u32 mark, mask;
};

/* From xt_mark.h (Match) */
struct xt_mark_mtinfo1 {
       __u32 mark, mask;
       __u8 invert;
};

#endif /*_XT_MARK_UAPI_H*/
