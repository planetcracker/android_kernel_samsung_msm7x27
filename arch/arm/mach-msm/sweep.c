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
#include <linux/input.h>

int sweeptowake;
int sweeptolock;
int sweepkeyone;
int sweepkeytwo;
int sweepkeythree;
int SKEY_ONE;
int SKEY_TWO;
int SKEY_THREE;

/* sysfs interface for "sweeptowake" */
#define show_one(file_name, object)					\
static ssize_t show_##file_name						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)          \
{									\
	return sprintf(buf, "%d\n", object);				\
}

show_one(sweeptowake, sweeptowake);
show_one(sweeptolock, sweeptolock);
show_one(sweepkeyone, sweepkeyone);
show_one(sweepkeytwo, sweepkeytwo);
show_one(sweepkeythree, sweepkeythree);
show_one(keycode_one, SKEY_ONE);
show_one(keycode_two, SKEY_TWO);
show_one(keycode_three, SKEY_THREE);

static ssize_t sweeptowake_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%du", &input);

	if (ret != 1 || input > 1 ||
			input < 0) {
		return -EINVAL;
	}

	sweeptowake = input;
	return count;
}
static ssize_t sweeptolock_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%du", &input);

	if (ret != 1 || input > 1 ||
			input < 0) {
		return -EINVAL;
	}

	sweeptolock = input;
	return count;
}
static ssize_t sweepkeyone_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%du", &input);

	if (ret != 1 || input > 1 ||
			input < 0) {
		return -EINVAL;
	}

	sweepkeyone = input;
	return count;
}
static ssize_t sweepkeytwo_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%du", &input);

	if (ret != 1 || input > 1 ||
			input < 0) {
		return -EINVAL;
	}

	sweepkeytwo = input;
	return count;
}
static ssize_t sweepkeythree_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%du", &input);

	if (ret != 1 || input > 1 ||
			input < 0) {
		return -EINVAL;
	}

	sweepkeythree = input;
	return count;
}

static ssize_t keycode_one_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%du", &input);

	if (ret != 1 || input > 247 ||
			input < 0) {
		return -EINVAL;
	}

	SKEY_ONE = input;
	return count;
}
static ssize_t keycode_two_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%du", &input);

	if (ret != 1 || input > 247 ||
			input < 0) {
		return -EINVAL;
	}

	SKEY_TWO = input;
	return count;
}
static ssize_t keycode_three_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%du", &input);

	if (ret != 1 || input > 247 ||
			input < 0) {
		return -EINVAL;
	}

	SKEY_THREE = input;
	return count;
}

#define define_kobj_rw_attr(_name)		\
static struct kobj_attribute _name##_attribute =	\
	__ATTR(_name, 0666, show_##_name, _name##_store)

define_kobj_rw_attr(sweeptowake);
define_kobj_rw_attr(sweeptolock);
define_kobj_rw_attr(sweepkeyone);
define_kobj_rw_attr(sweepkeytwo);
define_kobj_rw_attr(sweepkeythree);
define_kobj_rw_attr(keycode_one);
define_kobj_rw_attr(keycode_two);
define_kobj_rw_attr(keycode_three);

static struct attribute *sweeptowake_attrs[] = {
&sweeptowake_attribute.attr,
&sweeptolock_attribute.attr,
&sweepkeyone_attribute.attr,
&sweepkeytwo_attribute.attr,
&sweepkeythree_attribute.attr,
&keycode_one_attribute.attr,
&keycode_two_attribute.attr,
&keycode_three_attribute.attr,
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
	sweeptolock = 1; /* Sweep2lock enabled by default */
	sweepkeyone = 1;
	sweepkeytwo = 1;
	sweepkeythree = 1;
	SKEY_ONE = KEY_HOME;
	SKEY_TWO = KEY_HOME;
	SKEY_THREE = KEY_BACK;

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


