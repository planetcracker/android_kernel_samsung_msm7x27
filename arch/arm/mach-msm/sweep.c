/*
 * Author: Chad Froebel <chadfroebel@gmail.com>
 *
 * Simple port to Nexus 4 : motley <motley.slate@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * Possible values for "sweeptowake" are :
 *
 *   0 - disabled
 *   1 - enabled  (default)
*/

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/sweep.h>
#include <linux/module.h>

int sweeptowake;

/* sysfs interface for "sweeptowake" */
static ssize_t sweeptowake_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
return sprintf(buf, "%d\n", sweeptowake);
}

static ssize_t sweeptowake_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

int new_sweeptowake;

sscanf(buf, "%du", &new_sweeptowake);

if (new_sweeptowake >= 0 && new_sweeptowake <= 1) {
	/* update only if valid value provided */
	sweeptowake = new_sweeptowake;
}

return count;
}

static struct kobj_attribute sweeptowake_attribute =
__ATTR(sweeptowake, 0666, sweeptowake_show, sweeptowake_store);

static struct attribute *sweeptowake_attrs[] = {
&sweeptowake_attribute.attr,
NULL,
};

static struct attribute_group sweeptowake_attr_group = {
.attrs = sweeptowake_attrs,
};

/* Initialize sweep sysfs folder */
static struct kobject *sweeptowake_kobj;

static int __init sweep_init(void)
{
	int sweeptowake_retval;

	sweeptowake = 1; /* Sweep2Wake enabled by default */

	sweeptowake_kobj = kobject_create_and_add("sweep", kernel_kobj);
	if (!sweeptowake_kobj) {
			return -ENOMEM;
	}

	sweeptowake_retval = sysfs_create_group(sweeptowake_kobj, &sweeptowake_attr_group);

	if (sweeptowake_retval)
		kobject_put(sweeptowake_kobj);

	return (sweeptowake_retval);
}

static void __exit sweep_exit(void)
{
	kobject_put(sweeptowake_kobj);
}

module_init(sweep_init);
module_exit(sweep_exit);


