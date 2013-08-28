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

int pll2_overclock;

int aggressivity;

#define show_one(file_name, object)					\
static ssize_t show_##file_name						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)          \
{									\
	return sprintf(buf, "%d\n", object);				\
}

show_one(volt_66, pll0_960_pll1_245_pll2_1200[1].vdd);
show_one(volt_122, pll0_960_pll1_245_pll2_1200[3].vdd);
show_one(volt_245, pll0_960_pll1_245_pll2_1200[5].vdd);
show_one(volt_360, pll0_960_pll1_245_pll2_1200[6].vdd);
show_one(volt_480, pll0_960_pll1_245_pll2_1200[8].vdd);
show_one(volt_604, pll0_960_pll1_245_pll2_1200[9].vdd);
show_one(volt_658, pll0_960_pll1_245_pll2_1200[10].vdd);
#ifndef CONFIG_MACH_BENI
show_one(volt_770, pll0_960_pll1_245_pll2_1200[11].vdd);
#else
show_one(volt_770, pll0_960_pll1_245_pll2_1200[13].vdd);
#endif
show_one(pll2_overclock, pll2_overclock);
show_one(aggressivity, aggressivity);

static ssize_t volt_66_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > pll0_960_pll1_245_pll2_1200[3].vdd) {
		return -EINVAL;
	}

	pll0_960_pll1_245_pll2_1200[1].vdd = input;
	pll0_960_pll1_245_pll2_1200[2].vdd = input;
	return count;
}
static ssize_t volt_122_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input > pll0_960_pll1_245_pll2_1200[5].vdd || input < pll0_960_pll1_245_pll2_1200[1].vdd) {
		return -EINVAL;
	}

	pll0_960_pll1_245_pll2_1200[3].vdd = input;
	pll0_960_pll1_245_pll2_1200[4].vdd = input;
	return count;
}
static ssize_t volt_245_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > pll0_960_pll1_245_pll2_1200[6].vdd || input < pll0_960_pll1_245_pll2_1200[3].vdd) {
		return -EINVAL;
	}

	pll0_960_pll1_245_pll2_1200[5].vdd = input;
	return count;
}
static ssize_t volt_360_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > pll0_960_pll1_245_pll2_1200[8].vdd || input < pll0_960_pll1_245_pll2_1200[5].vdd) {
		return -EINVAL;
	}

	pll0_960_pll1_245_pll2_1200[6].vdd = input;
	return count;
}
static ssize_t volt_480_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > pll0_960_pll1_245_pll2_1200[9].vdd || input < pll0_960_pll1_245_pll2_1200[6].vdd) {
		return -EINVAL;
	}

	pll0_960_pll1_245_pll2_1200[7].vdd = input;
	pll0_960_pll1_245_pll2_1200[8].vdd = input;
	return count;
}
static ssize_t volt_604_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > pll0_960_pll1_245_pll2_1200[10].vdd || input < pll0_960_pll1_245_pll2_1200[8].vdd) {
		return -EINVAL;
	}

	pll0_960_pll1_245_pll2_1200[9].vdd = input;
	return count;
}
static ssize_t volt_658_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input > 7 || input < pll0_960_pll1_245_pll2_1200[9].vdd) {
		return -EINVAL;
	}

	pll0_960_pll1_245_pll2_1200[10].vdd = input;
#ifdef CONFIG_MACH_BENI
	pll0_960_pll1_245_pll2_1200[11].vdd = input;
#endif
	return count;
}
static ssize_t volt_770_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int input;
	int ret;
	ret = sscanf(buf, "%d", &input);

	if (ret != 1 || input < 0 || input < pll0_960_pll1_245_pll2_1200[10].vdd || input > 8) {
		return -EINVAL;
	}

#ifdef CONFIG_MACH_BENI
	pll0_960_pll1_245_pll2_1200[12].vdd = input;
	pll0_960_pll1_245_pll2_1200[13].vdd = input;
#else
	pll0_960_pll1_245_pll2_1200[11].vdd = input;
#endif
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

#ifndef CONFIG_MACH_BENI
	pll2_overclock = 1;
	pll0_960_pll1_245_pll2_1200[1].vdd = 0;
	pll0_960_pll1_245_pll2_1200[2].vdd = 0;
	pll0_960_pll1_245_pll2_1200[3].vdd = 0;
	pll0_960_pll1_245_pll2_1200[4].vdd = 1;
	pll0_960_pll1_245_pll2_1200[5].vdd = 1;
	pll0_960_pll1_245_pll2_1200[6].vdd = 2;
	pll0_960_pll1_245_pll2_1200[7].vdd = 3;
	pll0_960_pll1_245_pll2_1200[8].vdd = 3;
	pll0_960_pll1_245_pll2_1200[9].vdd = 4;
	pll0_960_pll1_245_pll2_1200[10].vdd = 5;
	pll0_960_pll1_245_pll2_1200[11].vdd = 5;
#else
	pll2_overclock = 0;
	pll0_960_pll1_245_pll2_1200[1].vdd = 0;
	pll0_960_pll1_245_pll2_1200[2].vdd = 0;
	pll0_960_pll1_245_pll2_1200[3].vdd = 1;
	pll0_960_pll1_245_pll2_1200[4].vdd = 2;
	pll0_960_pll1_245_pll2_1200[5].vdd = 2;
	pll0_960_pll1_245_pll2_1200[6].vdd = 3;
	pll0_960_pll1_245_pll2_1200[7].vdd = 4;
	pll0_960_pll1_245_pll2_1200[8].vdd = 4;
	pll0_960_pll1_245_pll2_1200[9].vdd = 5;
	pll0_960_pll1_245_pll2_1200[10].vdd = 6;
	pll0_960_pll1_245_pll2_1200[11].vdd = 6;
	pll0_960_pll1_245_pll2_1200[12].vdd = 7;
	pll0_960_pll1_245_pll2_1200[13].vdd = 7;
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


