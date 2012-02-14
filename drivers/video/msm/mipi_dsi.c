/* Copyright (c) 2008-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <mach/clk.h>

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mdp.h"
#include "mdp4.h"

static int mipi_dsi_probe(struct platform_device *pdev);
static int mipi_dsi_remove(struct platform_device *pdev);

static int mipi_dsi_off(struct platform_device *pdev);
static int mipi_dsi_on(struct platform_device *pdev);

static struct clk *dsi_byte_div_clk;
static struct clk *dsi_esc_clk;
static struct clk *dsi_m_pclk;
static struct clk *dsi_s_pclk;
static struct clk *amp_pclk;

static char *mmss_cc_base;	/* mutimedia sub system clock control */
static char *mmss_sfpb_base;	/* mutimedia sub system sfpb */

static struct platform_device *pdev_list[MSM_FB_MAX_DEV_LIST];
static int pdev_list_cnt;
static struct mipi_dsi_platform_data *mipi_dsi_pdata;

static int vsync_gpio = -1;

static struct platform_driver mipi_dsi_driver = {
	.probe = mipi_dsi_probe,
	.remove = mipi_dsi_remove,
	.shutdown = NULL,
	.driver = {
		   .name = "mipi_dsi",
		   },
};

struct device dsi_dev;

/* MIPI_DSI_MRPS, Maximum Return Packet Size */
char max_pktsize[2] = {MIPI_DSI_MRPS, 0x00}; /* LSB tx first, 16 bytes */

/*
 * mipi_dsi_mxo_selected() -
 *
 *	determine if PXO or MXO should be used as clk_ref for the
 *	DSI PLL.
 *
 *	Note - this function can only be called after clk_enable()
 *	is called for the dsi_esc_clk, as that is what ensures that
 *	the MXO/PXO slection has been made for the dsi_esc_clk.  This
 *	function simply reads what the kernel's clock manager has
 *	configured for dsi_esc_clk.
 *
 *	Returns 1 if MXO should be used, 0 if PXO should be used
 *
 */
static int mipi_dsi_mxo_selected(void)
{
	uint32_t *cc;
	uint32 data;

	cc = (uint32 *)(mmss_cc_base + 0x004c);
	data = MIPI_INP(cc);

	if (data & BIT(14))
		return 1;
	else
		return 0;
}


#ifdef DSI_CLK_CALCULATE
static void mipi_dsi_clk(int on, struct dsi_clk_desc *clk)
{
	uint32 *cc, *ns, *md;
	uint32 data, val;

	cc = (uint32 *)(mmss_cc_base + 0x004c);
	md = (uint32 *)(mmss_cc_base + 0x0050);
	ns = (uint32 *)(mmss_cc_base + 0x0054);

	val = clk->d * 2;
	data = (~val) & 0x0ff;
	data |= clk->m << 8;
	MIPI_OUTP(md, data);

	val = clk->n - clk->m;
	data = (~val) & 0x0ff;
	data <<= 24;
	data |= clk->src;
	MIPI_OUTP(ns, data);

	/*
	 * Desctiption: change to DSI_CMD_MODE since it needed to
	 * tx DCS dsiplay off comamnd to panel
	 */
	mipi_dsi_op_mode_config(DSI_CMD_MODE);

	if (pinfo->lcd.vsync_enable) {
		if (pinfo->lcd.hw_vsync_mode && vsync_gpio > 0)
			gpio_free(vsync_gpio);

		mipi_dsi_set_tear_off(mfd);
	}

	ret = panel_next_off(pdev);

	mutex_lock(&mfd->dma->ov_mutex);
	/* make sure mdp dma is not running */
	mdp4_dsi_cmd_dma_busy_wait(mfd);

#ifdef CONFIG_MSM_BUS_SCALING
	mdp_bus_scale_update_request(0);
#else
	if (mfd->ebi1_clk)
		clk_disable(mfd->ebi1_clk);
#endif

	disable_irq(DSI_IRQ);

	/* disbale dsi pll */
	MIPI_OUTP(MIPI_DSI_BASE + 0x0200, 0);

	/* disbale dsi clk */
	MIPI_OUTP(MIPI_DSI_BASE + 0x0118, 0);

	mipi_dsi_pclk(0);
	mipi_dsi_clk(0);
	clk_disable(dsi_esc_clk);
	clk_disable(dsi_byte_div_clk);
	clk_disable(dsi_m_pclk);
	clk_disable(dsi_s_pclk);
	clk_disable(amp_pclk); /* clock for AHB-master to AXI */

	/* disbale dsi engine */
	MIPI_OUTP(MIPI_DSI_BASE + 0x0000, 0);

	if (mipi_dsi_pdata && mipi_dsi_pdata->dsi_power_save)
		mipi_dsi_pdata->dsi_power_save(0);

	mutex_unlock(&mfd->dma->ov_mutex);

	pr_debug("%s:\n", __func__);

	return ret;
}

static int mipi_dsi_on(struct platform_device *pdev)
{
	int ret = 0;
	u32 clk_rate;
	struct msm_fb_data_type *mfd;
	struct fb_info *fbi;
	struct fb_var_screeninfo *var;
	struct msm_panel_info *pinfo;
	struct mipi_panel_info *mipi;
	u32 hbp, hfp, vbp, vfp, hspw, vspw, width, height;
	u32 ystride, bpp, data;

	mfd = platform_get_drvdata(pdev);
	fbi = mfd->fbi;
	var = &fbi->var;
	pinfo = &mfd->panel_info;

	if (mipi_dsi_pdata && mipi_dsi_pdata->dsi_power_save)
		mipi_dsi_pdata->dsi_power_save(1);

	clk_rate = mfd->fbi->var.pixclock;
	clk_rate = min(clk_rate, mfd->panel_info.clk_max);

	mipi_dsi_phy_ctrl(1);

	if (mdp_rev == MDP_REV_42 && mipi_dsi_pdata)
		target_type = mipi_dsi_pdata->target_type;

	mipi_dsi_phy_init(0, &(mfd->panel_info), target_type);

	local_bh_disable();
	mipi_dsi_clk_enable();
	local_bh_enable();

	MIPI_OUTP(MIPI_DSI_BASE + 0x114, 1);
	MIPI_OUTP(MIPI_DSI_BASE + 0x114, 0);

	hbp = var->left_margin;
	hfp = var->right_margin;
	vbp = var->upper_margin;
	vfp = var->lower_margin;
	hspw = var->hsync_len;
	vspw = var->vsync_len;
	width = mfd->panel_info.xres;
	height = mfd->panel_info.yres;

	mipi  = &mfd->panel_info.mipi;

	if (mfd->panel_info.type == MIPI_VIDEO_PANEL) {
		MIPI_OUTP(MIPI_DSI_BASE + 0x20,
			((hbp + width - 1) << 16 | (hbp - 1)));
		MIPI_OUTP(MIPI_DSI_BASE + 0x24,
			((vbp + height - 1) << 16 | (vbp - 1)));
		MIPI_OUTP(MIPI_DSI_BASE + 0x28,
			(vbp + height + vfp - 1) << 16 |
				(hbp + width + hfp - 1));
		MIPI_OUTP(MIPI_DSI_BASE + 0x2c, (hspw - 1) << 16);
		MIPI_OUTP(MIPI_DSI_BASE + 0x30, 0);
		MIPI_OUTP(MIPI_DSI_BASE + 0x34, (vspw - 1) << 16);

	} else {		/* command mode */
		if (mipi->dst_format == DSI_CMD_DST_FORMAT_RGB888)
			bpp = 3;
		else if (mipi->dst_format == DSI_CMD_DST_FORMAT_RGB666)
			bpp = 3;
		else if (mipi->dst_format == DSI_CMD_DST_FORMAT_RGB565)
			bpp = 2;
		else
			bpp = 1;

		ystride = width * bpp + 1;

		/* DSI_COMMAND_MODE_MDP_STREAM_CTRL */
		data = (ystride << 16) | (mipi->vc << 8) | DTYPE_DCS_LWRITE;
		MIPI_OUTP(MIPI_DSI_BASE + 0x5c, data);
		MIPI_OUTP(MIPI_DSI_BASE + 0x54, data);

		/* DSI_COMMAND_MODE_MDP_STREAM_TOTAL */
		data = height << 16 | width;
		MIPI_OUTP(MIPI_DSI_BASE + 0x60, data);
		MIPI_OUTP(MIPI_DSI_BASE + 0x58, data);
	}

	mipi_dsi_host_init(mipi);

	mipi_dsi_cmd_bta_sw_trigger(); /* clean up ack_err_status */

	ret = panel_next_on(pdev);

	mipi_dsi_op_mode_config(mipi->mode);

	if (mfd->panel_info.type == MIPI_CMD_PANEL) {
		if (pinfo->lcd.vsync_enable) {
			if (pinfo->lcd.hw_vsync_mode && vsync_gpio > 0) {
				if (gpio_request(vsync_gpio, "MDP_VSYNC") == 0)
					gpio_direction_input(vsync_gpio);
				else
					pr_err("%s: unable to request gpio=%d\n",
						__func__, vsync_gpio);
			}
			mipi_dsi_set_tear_on(mfd);
		}
	}

#ifdef CONFIG_MSM_BUS_SCALING
	mdp_bus_scale_update_request(2);
#else
	if (mfd->ebi1_clk)
		clk_enable(mfd->ebi1_clk);
#endif

	pr_debug("%s:\n", __func__);

	return ret;
}

static int mipi_dsi_resource_initialized;

static int mipi_dsi_probe(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct fb_info *fbi;
	struct msm_panel_info *pinfo;
	struct platform_device *mdp_dev = NULL;
	struct msm_fb_panel_data *pdata = NULL;
	int rc;
	resource_size_t size ;

	if ((pdev->id == 0) && (pdev->num_resources >= 0)) {
		mipi_dsi_pdata = pdev->dev.platform_data;

		size =  resource_size(&pdev->resource[0]);
		mipi_dsi_base =  ioremap(pdev->resource[0].start, size);

		MSM_FB_INFO("mipi_dsi base phy_addr = 0x%x virt = 0x%x\n",
				pdev->resource[0].start, (int) mipi_dsi_base);

		if (!mipi_dsi_base)
			return -ENOMEM;

		mmss_cc_base =  ioremap(MMSS_CC_BASE_PHY, 0x200);
		MSM_FB_INFO("mmss_cc base phy_addr = 0x%x virt = 0x%x\n",
				MMSS_CC_BASE_PHY, (int) mmss_cc_base);

		if (!mmss_cc_base)
			return -ENOMEM;

		mmss_sfpb_base =  ioremap(MMSS_SFPB_BASE_PHY, 0x100);
		MSM_FB_INFO("mmss_sfpb  base phy_addr = 0x%x virt = 0x%x\n",
				MMSS_SFPB_BASE_PHY, (int) mmss_sfpb_base);

		if (!mmss_cc_base)
			return -ENOMEM;

		rc = request_irq(DSI_IRQ, mipi_dsi_isr, IRQF_DISABLED,
						"MIPI_DSI", 0);
		if (rc) {
			printk(KERN_ERR "mipi_dsi_host request_irq() failed!\n");
			return rc;
		}

		disable_irq(DSI_IRQ);

		mipi_dsi_calibration();

		if (mipi_dsi_pdata) {
			vsync_gpio = mipi_dsi_pdata->vsync_gpio;
			pr_info("%s: vsync_gpio=%d\n", __func__, vsync_gpio);
		}

		mipi_dsi_resource_initialized = 1;

		return 0;
	}

	mipi_dsi_clk_init(pdev);

	if (!mipi_dsi_resource_initialized)
		return -EPERM;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if (pdev_list_cnt >= MSM_FB_MAX_DEV_LIST)
		return -ENOMEM;


	mdp_dev = platform_device_alloc("mdp", pdev->id);
	if (!mdp_dev)
		return -ENOMEM;

	/*
	 * link to the latest pdev
	 */
	mfd->pdev = mdp_dev;
	mfd->dest = DISPLAY_LCD;

	/*
	 * alloc panel device data
	 */
	if (platform_device_add_data
	    (mdp_dev, pdev->dev.platform_data,
	     sizeof(struct msm_fb_panel_data))) {
		printk(KERN_ERR "mipi_dsi_probe: platform_device_add_data failed!\n");
		platform_device_put(mdp_dev);
		return -ENOMEM;
	}
	/*
	 * data chain
	 */
	pdata = mdp_dev->dev.platform_data;
	pdata->on = mipi_dsi_on;
	pdata->off = mipi_dsi_off;
	pdata->next = pdev;

	/*
	 * get/set panel specific fb info
	 */
	mfd->panel_info = pdata->panel_info;
	pinfo = &mfd->panel_info;

	if (mfd->index == 0)
		mfd->fb_imgType = MSMFB_DEFAULT_TYPE;
	else
		mfd->fb_imgType = MDP_RGB_565;

	fbi = mfd->fbi;
	fbi->var.pixclock = mfd->panel_info.clk_rate;
	fbi->var.left_margin = mfd->panel_info.lcdc.h_back_porch;
	fbi->var.right_margin = mfd->panel_info.lcdc.h_front_porch;
	fbi->var.upper_margin = mfd->panel_info.lcdc.v_back_porch;
	fbi->var.lower_margin = mfd->panel_info.lcdc.v_front_porch;
	fbi->var.hsync_len = mfd->panel_info.lcdc.h_pulse_width;
	fbi->var.vsync_len = mfd->panel_info.lcdc.v_pulse_width;

#ifdef DSI_CLK
	clk_rate = mfd->panel_info.clk_max;
	if (clk_set_max_rate(mipi_dsi_clk, clk_rate) < 0)
		printk(KERN_ERR "%s: clk_set_max_rate failed\n", __func__);
	mfd->panel_info.clk_rate = mfd->panel_info.clk_min;
#endif

	/*
	 * set driver data
	 */
	platform_set_drvdata(mdp_dev, mfd);

	/*
	 * register in mdp driver
	 */
	rc = platform_device_add(mdp_dev);
	if (rc)
		goto mipi_dsi_probe_err;

	pdev_list[pdev_list_cnt++] = pdev;

#ifndef CONFIG_MSM_BUS_SCALING
	if (IS_ERR(mfd->ebi1_clk)) {
		rc = PTR_ERR(mfd->ebi1_clk);
		goto mipi_dsi_probe_err;
	}
	clk_set_rate(mfd->ebi1_clk, 122000000);
#endif

	return 0;

mipi_dsi_probe_err:
	platform_device_put(mdp_dev);
	return rc;
}

static int mipi_dsi_remove(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);
#ifndef CONFIG_MSM_BUS_SCALING
	clk_put(mfd->ebi1_clk);
#endif

	iounmap(msm_pmdh_base);

	return 0;
}

static int mipi_dsi_register_driver(void)
{
	return platform_driver_register(&mipi_dsi_driver);
}

static int __init mipi_dsi_driver_init(void)
{
	int ret;

	amp_pclk = clk_get(NULL, "amp_pclk");
	if (IS_ERR(amp_pclk)) {
		printk(KERN_ERR "can't find amp_pclk\n");
		return PTR_ERR(amp_pclk);
	}

	dsi_m_pclk = clk_get(NULL, "dsi_m_pclk");
	if (IS_ERR(dsi_m_pclk)) {
		printk(KERN_ERR "can't find dsi_m_pclk\n");
		return PTR_ERR(dsi_m_pclk);
	}

	dsi_s_pclk = clk_get(NULL, "dsi_s_pclk");
	if (IS_ERR(dsi_s_pclk)) {
		printk(KERN_ERR "can't find dsi_s_pclk\n");
		return PTR_ERR(dsi_s_pclk);
	}

	dsi_byte_div_clk = clk_get(NULL, "dsi_byte_div_clk");
	if (IS_ERR(dsi_byte_div_clk)) {
		printk(KERN_ERR "can't find dsi_byte_div_clk\n");
		return PTR_ERR(dsi_byte_div_clk);
	}


	dsi_esc_clk = clk_get(NULL, "dsi_esc_clk");
	if (IS_ERR(dsi_esc_clk)) {
		printk(KERN_ERR "can't find dsi_byte_div_clk\n");
		return PTR_ERR(dsi_byte_div_clk);
	}

	ret = mipi_dsi_register_driver();

	device_initialize(&dsi_dev);

	if (ret) {
		clk_disable(amp_pclk);
		clk_put(amp_pclk);
		clk_disable(dsi_m_pclk);
		clk_put(dsi_m_pclk);
		clk_disable(dsi_s_pclk);
		clk_put(dsi_s_pclk);
		clk_disable(dsi_byte_div_clk);
		clk_put(dsi_byte_div_clk);
		clk_disable(dsi_esc_clk);
		clk_put(dsi_esc_clk);
		printk(KERN_ERR "mipi_dsi_register_driver() failed!\n");
		return ret;
	}

	mipi_dsi_init();

	return ret;
}

module_init(mipi_dsi_driver_init);
