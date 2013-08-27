/*
 * Author: Alin Panaitiu <alin.p32@gmail.com>
 *
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

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/perf_control.h>
#include <linux/module.h>
#include <linux/input.h>

int volt_66;
int volt_122;
int volt_245;
int volt_360;
int volt_480;
int volt_604;
int volt_658;
int volt_770;
int pll2_overclock;

int aggressivity;

#define show_one(file_name, object)					\
static ssize_t show_##file_name						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)          \
{									\
	return sprintf(buf, "%d\n", object);				\
}

show_one(volt_66, volt_66);
show_one(volt_122, volt_122);
show_one(volt_245, volt_245);
show_one(volt_360, volt_360);
show_one(volt_480, volt_480);
show_one(volt_604, volt_604);
show_one(volt_658, volt_658);
show_one(volt_770, volt_770);
show_one(pll2_overclock, pll2_overclock);
show_one(aggressivity, aggressivity);

static ssize_t volt_66_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > volt_122) {
		return -EINVAL;
	}

	volt_66 = input;
	voltage_levels(66);
	return count;
}
static ssize_t volt_122_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > volt_245 || input < volt_66) {
		return -EINVAL;
	}

	volt_122 = input;
	voltage_levels(122);
	return count;
}
static ssize_t volt_245_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > volt_360 || input < volt_122) {
		return -EINVAL;
	}

	volt_245 = input;
	voltage_levels(245);
	return count;
}
static ssize_t volt_360_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > volt_480 || input < volt_245) {
		return -EINVAL;
	}

	volt_360 = input;
	voltage_levels(360);
	return count;
}
static ssize_t volt_480_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > volt_604 || input < volt_360) {
		return -EINVAL;
	}

	volt_480 = input;
	voltage_levels(480);
	return count;
}
static ssize_t volt_604_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > volt_658 || input < volt_480) {
		return -EINVAL;
	}

	volt_604 = input;
	voltage_levels(604);
	return count;
}
static ssize_t volt_658_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > volt_770 || input < volt_604) {
		return -EINVAL;
	}

	volt_658 = input;
	voltage_levels(658);
	return count;
}
static ssize_t volt_770_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input < volt_658 || input > 8) {
		return -EINVAL;
	}

	volt_770 = input;
	voltage_levels(770);
	return count;
}
static ssize_t pll2_overclock_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > 1) {
		return -EINVAL;
	}

	pll2_overclock = input;
	return count;
}
static ssize_t aggressivity_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > 10) {
		return -EINVAL;
	}

	aggressivity = input;
	lowmem_compute(aggressivity);
	return count;
}

#define define_kobj_rw_attr(_name)		\
static struct kobj_attribute _name##_attribute =	\
	__ATTR(_name, 0666, show_##_name, _name##_store)

define_kobj_rw_attr(volt_66);
define_kobj_rw_attr(volt_122);
define_kobj_rw_attr(volt_245);
define_kobj_rw_attr(volt_360);
define_kobj_rw_attr(volt_480);
define_kobj_rw_attr(volt_604);
define_kobj_rw_attr(volt_658);
define_kobj_rw_attr(volt_770);
define_kobj_rw_attr(pll2_overclock);
define_kobj_rw_attr(aggressivity);

static struct attribute *perform_c_attrs[] = {
&volt_66_attribute.attr,
&volt_122_attribute.attr,
&volt_245_attribute.attr,
&volt_360_attribute.attr,
&volt_480_attribute.attr,
&volt_604_attribute.attr,
&volt_658_attribute.attr,
&volt_770_attribute.attr,
&pll2_overclock_attribute.attr,
&aggressivity_attribute.attr,
NULL,
};

static struct attribute_group perform_c_attr_group = {
.attrs = perform_c_attrs,
};

static struct kobject *perform_c_kobj;

static int __init perform_c_init(void)
{
	int perform_c_retval;

	volt_66 = 0;
	volt_122 = 1;
	volt_245 = 2;
	volt_360 = 3;
	volt_480 = 4;
	volt_604 = 5;
	volt_658 = 6;
	volt_770 = 7;
#ifdef CONFIG_MACH_BENI
	pll2_overclock = 0;
#else
	pll2_overclock = 1;
#endif
	aggressivity = 5;

	perform_c_kobj = kobject_create_and_add("performance_control", kernel_kobj);
	if (!perform_c_kobj) {
			return -ENOMEM;
	}

	perform_c_retval = sysfs_create_group(perform_c_kobj, &perform_c_attr_group);

	if (perform_c_retval)
		kobject_put(perform_c_kobj);

	return (perform_c_retval);
}

static void __exit perform_c_exit(void)
{
	kobject_put(perform_c_kobj);
}

module_init(perform_c_init);
module_exit(perform_c_exit);


