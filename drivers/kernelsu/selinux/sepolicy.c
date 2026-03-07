#include <linux/ratelimit.h>
#include <linux/version.h>
#include <linux/printk.h>
#include "selinux.h"
#include "sepolicy.h"
#include "ss/policydb.h"
#include "ss/services.h"

// KernelSU Policy Implementation Stubs for 4.19
// On 4.19 kernels, SELinux internal structures are significantly different and not easily patchable.
// These stubs allow the kernel to compile while disabling in-kernel SELinux patching for KernelSU.

bool ksu_load_policy(void)
{
    return false;
}

void ksu_setenforce(bool enforce)
{
    pr_info("KernelSU: setenforce %d (stub)\n", enforce);
}

bool ksu_is_enforce(void)
{
    return true;
}

int ksu_apply_policy(struct policydb *db)
{
    return 0;
}

bool ksu_type(struct policydb *db, const char *name, const char *attr)
{
    return false;
}

bool ksu_attribute(struct policydb *db, const char *name)
{
    return false;
}

bool ksu_permissive(struct policydb *db, const char *type)
{
    return false;
}

bool ksu_enforce(struct policydb *db, const char *type)
{
    return false;
}

bool ksu_typeattribute(struct policydb *db, const char *type, const char *attr)
{
    return false;
}

bool ksu_exists(struct policydb *db, const char *type)
{
    return false;
}

bool ksu_allow(struct policydb *db, const char *src, const char *tgt,
               const char *cls, const char *perm)
{
    return false;
}

bool ksu_deny(struct policydb *db, const char *src, const char *tgt,
              const char *cls, const char *perm)
{
    return false;
}

bool ksu_auditallow(struct policydb *db, const char *src, const char *tgt,
                    const char *cls, const char *perm)
{
    return false;
}

bool ksu_dontaudit(struct policydb *db, const char *src, const char *tgt,
                   const char *cls, const char *perm)
{
    return false;
}

bool ksu_allowxperm(struct policydb *db, const char *src, const char *tgt,
                    const char *cls, const char *range)
{
    return false;
}

bool ksu_auditallowxperm(struct policydb *db, const char *src, const char *tgt,
                         const char *cls, const char *range)
{
    return false;
}

bool ksu_dontauditxperm(struct policydb *db, const char *src, const char *tgt,
                        const char *cls, const char *range)
{
    return false;
}

bool ksu_type_transition(struct policydb *db, const char *src, const char *tgt,
                         const char *cls, const char *def, const char *obj)
{
    return false;
}

bool ksu_type_change(struct policydb *db, const char *src, const char *tgt,
                     const char *cls, const char *def)
{
    return false;
}

bool ksu_type_member(struct policydb *db, const char *src, const char *tgt,
                     const char *cls, const char *def)
{
    return false;
}

bool ksu_genfscon(struct policydb *db, const char *fs_name, const char *path,
                  const char *ctx)
{
    return false;
}
