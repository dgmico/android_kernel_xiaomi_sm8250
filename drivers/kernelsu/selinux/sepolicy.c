#include <linux/ratelimit.h>
#include <linux/version.h>
#include "selinux.h"
#include "sepolicy.h"
#include "ss/policydb.h"
#include "ss/services.h"
#include "objsec.h"

// KernelSU Policy Implementation
// Stubs for older kernels (like 4.19) where SELinux structures are significantly different.

bool ksu_load_policy(void)
{
	return false;
}

void ksu_setenforce(bool enforce)
{
	pr_info("KernelSU: setenforce %d\n", enforce);
}

bool ksu_is_enforce(void)
{
	return true;
}

static bool add_rule(struct policydb *db, const char *s, const char *t,
		     const char *c, const char *p, int effect)
{
	return false;
}

static bool add_type(struct policydb *db, const char *type_name)
{
	return false;
}

static bool add_filename_trans(struct policydb *db, const char *s,
			       const char *t, const char *c, const char *d,
			       const char *o)
{
	return false;
}

static bool add_genfscon(struct policydb *db, const char *fs_name,
			 const char *path, const char *context)
{
	return false;
}

int ksu_apply_policy(struct policydb *db)
{
	return 0;
}
