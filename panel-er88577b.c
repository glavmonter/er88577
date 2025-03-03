#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_connector.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_print.h>

#include "drm_mipi_dsi_ex.h"
#include "drm_probe_helper_ex.h"

#define DSI_DRIVER_NAME "panel-er88577b"

enum cmd_type {
	CMD_TYPE_DCS,
	CMD_TYPE_DELAY,
};

struct er88577b_init_cmd {
	enum cmd_type type;
	const char *data;
	size_t len;
};

#define _INIT_CMD_DCS(...)					\
	{							\
		.type	= CMD_TYPE_DCS,				\
		.data	= (char[]){__VA_ARGS__},		\
		.len	= sizeof((char[]){__VA_ARGS__})		\
	}							\

#define _INIT_CMD_DELAY(...)					\
	{							\
		.type	= CMD_TYPE_DELAY,			\
		.data	= (char[]){__VA_ARGS__},		\
		.len	= sizeof((char[]){__VA_ARGS__})		\
	}							\


struct er88577b_panel_desc {
	const struct drm_display_mode mode;
	unsigned int lanes;
	enum mipi_dsi_pixel_format format;
	unsigned long mode_flags;
	const struct er88577b_init_cmd *init_cmds;
	u32 num_init_cmds;
};

struct er88577b {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	const struct er88577b_panel_desc *desc;

	struct gpio_desc *reset;
	enum drm_panel_orientation orientation;
	struct regulator *power;
	bool bsit;
	bool debug;
};

#if 0
struct boe_th101mb31ig002;

//#define IS_ORIGINAL // Uncomment if use original config from linux-6.12 

struct panel_desc {
	const struct drm_display_mode *modes;
	unsigned long mode_flags;
	enum mipi_dsi_pixel_format format;
	int (*init)(struct boe_th101mb31ig002 *ctx);
	unsigned int lanes;
	bool lp11_before_reset;
	unsigned int vcioo_to_lp11_delay_ms;
	unsigned int lp11_to_reset_delay_ms;
	unsigned int backlight_off_to_display_off_delay_ms;
	unsigned int enter_sleep_to_reset_down_delay_ms;
	unsigned int power_off_delay_ms;
};

struct boe_th101mb31ig002 {
	struct drm_panel panel;

	struct mipi_dsi_device *dsi;

	const struct panel_desc *desc;

	struct regulator *power;
	// struct gpio_desc *enable;
	struct gpio_desc *reset;

	enum drm_panel_orientation orientation;
};


static void boe_th101mb31ig002_reset(struct boe_th101mb31ig002 *ctx)
{
	gpiod_direction_output(ctx->reset, 0);
	usleep_range(10, 100);
	gpiod_direction_output(ctx->reset, 1);
	usleep_range(10, 100);
	gpiod_direction_output(ctx->reset, 0);
	usleep_range(5000, 6000);
}

static int boe_th101mb31ig002_enable(struct boe_th101mb31ig002 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0xab, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0xba, 0xab);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1, 0x10, 0x01, 0x47, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0x0c, 0x14, 0x04, 0x50, 0x50, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3, 0x56, 0x53, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4, 0x33, 0x30, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6, 0xb0, 0x00, 0x00, 0x10, 0x00, 0x10,
					       0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8, 0x05, 0x12, 0x29, 0x49, 0x48, 0x00,
					       0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x7c, 0x65, 0x55, 0x49, 0x46, 0x36,
					       0x3b, 0x24, 0x3d, 0x3c, 0x3d, 0x5c, 0x4c,
					       0x55, 0x47, 0x46, 0x39, 0x26, 0x06, 0x7c,
					       0x65, 0x55, 0x49, 0x46, 0x36, 0x3b, 0x24,
					       0x3d, 0x3c, 0x3d, 0x5c, 0x4c, 0x55, 0x47,
					       0x46, 0x39, 0x26, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0xff, 0x87, 0x12, 0x34, 0x44, 0x44,
					       0x44, 0x44, 0x98, 0x04, 0x98, 0x04, 0x0f,
					       0x00, 0x00, 0xc1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1, 0x54, 0x94, 0x02, 0x85, 0x9f, 0x00,
					       0x7f, 0x00, 0x54, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x17, 0x09, 0x08, 0x89, 0x08, 0x11,
					       0x22, 0x20, 0x44, 0xff, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x86, 0x46, 0x05, 0x05, 0x1c, 0x1c,
					       0x1d, 0x1d, 0x02, 0x1f, 0x1f, 0x1e, 0x1e,
					       0x0f, 0x0f, 0x0d, 0x0d, 0x13, 0x13, 0x11,
					       0x11, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x07, 0x07, 0x04, 0x04, 0x1c, 0x1c,
					       0x1d, 0x1d, 0x02, 0x1f, 0x1f, 0x1e, 0x1e,
					       0x0e, 0x0e, 0x0c, 0x0c, 0x12, 0x12, 0x10,
					       0x10, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x2a, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x21, 0x00, 0x31, 0x42, 0x34, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0xcb, 0x43);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0x0e, 0x4b, 0x4b, 0x20, 0x19, 0x6b,
					       0x06, 0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0xe3, 0x2b, 0x38, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0x00, 0x01, 0x00, 0x0e, 0x04, 0x44,
					       0x08, 0x10, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0x80, 0x01, 0xff, 0xff, 0xff, 0xff,
					       0xff, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x12, 0x03, 0x20, 0x00, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf3, 0x00);

	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);

	mipi_dsi_msleep(&dsi_ctx, 150);

	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);

	return dsi_ctx.accum_err;
}

#ifdef IS_ORIGINAL
static int starry_er88577_init_cmd(struct boe_th101mb31ig002 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	msleep(70);

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx,  0xe0, 0xab, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx,  0xe1, 0xba, 0xab);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx,  0xb1, 0x10, 0x01, 0x47, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx,  0xb2, 0x0c, 0x14, 0x04, 0x50, 0x50, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx,  0xb3, 0x56, 0x53, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xb4, 0x33, 0x30, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xb6, 0xb0, 0x00, 0x00, 0x10, 0x00, 0x10, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xb8, 0x05, 0x12, 0x29, 0x49, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xb9, 0x7c, 0x61, 0x4f, 0x42, 0x3e, 0x2d, 0x31, 
											0x1a, 0x33, 0x33, 0x33, 0x52, 0x40, 0x47, 0x38, 
											0x34, 0x26, 0x0e, 0x06, 0x7c, 0x61, 0x4f, 0x42, 
											0x3e, 0x2d, 0x31, 0x1a, 0x33, 0x33, 0x33, 0x52, 
											0x40, 0x47, 0x38, 0x34, 0x26, 0x0e, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xc0, 0xcc, 0x76, 0x12, 0x34, 0x44, 0x44, 0x44, 
											0x44, 0x98, 0x04, 0x98, 0x04, 0x0f, 0x00, 0x00, 
											0xc1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xc1, 0x54, 0x94, 0x02, 0x85, 0x9f, 0x00, 0x6f, 
											0x00, 0x54, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xc2, 0x17, 0x09, 0x08, 0x89, 0x08, 0x11, 0x22, 
											0x20, 0x44, 0xff, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xc3, 0x87, 0x47, 0x05, 0x05, 0x1c, 0x1c, 0x1d,
											0x1d, 0x02, 0x1e, 0x1e, 0x1f, 0x1f, 0x0f, 0x0f, 
											0x0d, 0x0d, 0x13, 0x13, 0x11, 0x11, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xc4, 0x06, 0x06, 0x04, 0x04, 0x1c, 0x1c, 0x1d,
										   	0x1d, 0x02, 0x1e, 0x1e, 0x1f, 0x1f, 0x0e, 0x0e,  
										   	0x10, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xc8, 0x21, 0x00, 0x31, 0x42, 0x34, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xca, 0xcb, 0x43);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xcd, 0x0e, 0x4b, 0x4b, 0x20, 0x19, 0x6b, 0x06,
											0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xd1, 0x40, 0x0d, 0xff, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xd2, 0xe3, 0x2b, 0x38, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xd3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x20,
											0x3a, 0xd5, 0x86, 0xf3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xd4, 0x00, 0x01, 0x00, 0x0e, 0x04, 0x44, 0x08, 
											0x10, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xe6, 0x80, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xf0, 0x12, 0x03, 0x20, 0x00, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xf3, 0x00);


	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx); // DCS_Short_Write_NP(0x11);
	mipi_dsi_msleep(&dsi_ctx, 150);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);  // DCS_Short_Write_NP(0x29);
	mipi_dsi_msleep(&dsi_ctx, 20);

	return dsi_ctx.accum_err;
}

#else
static int starry_er88577_init_cmd(struct boe_th101mb31ig002 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	msleep(70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xE0, 0xAB, 0xBA);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xE1, 0xBA, 0xAB);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xB1, 0x10, 0x01, 0x47, 0xFF);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xB2, 0x0C, 0x14, 0x04, 0x50, 0x50, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xB3, 0x56, 0xD3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xB4, 0x22, 0x30, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xB6, 0xB0, 0x00, 0x00, 0x10, 0x00, 0x10, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xB7, 0x0E, 0x00, 0xFF, 0x08, 0x08, 0xFF, 0xFF, 
												  0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xB8, 0x05, 0x12, 0x29, 0x49, 0x48);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xB9, 0x7F, 0x69, 0x57, 0x4C, 0x47, 0x37, 0x3C, 
												  0x25, 0x3E, 0x3C, 0x3B, 0x58, 0x45, 0x4D, 0x40, 
												  0x3F, 0x35, 0x27, 0x06, 0x7F, 0x69, 0x57, 0x4C, 
												  0x47, 0x37, 0x3C, 0x25, 0x3E, 0x3C, 0x3B, 0x58, 
												  0x45, 0x4D, 0x40, 0x3F, 0x35, 0x27, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xC0, 0x98, 0x76, 0x12, 0x34, 0x33, 0x33, 0x44, 
												  0x44, 0x06, 0x04, 0x8A, 0x04, 0x0F, 0x00, 0x00, 
												  0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xC1, 0x53, 0x94, 0x02, 0x85, 0x06, 0x04, 0x8A, 
											      0x04, 0x54, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xC2, 0x37, 0x09, 0x08, 0x89, 0x08, 0x11, 0x22, 
												  0x21, 0x44, 0xBB, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xC3, 0x9C, 0x1D, 0x1E, 0x1F, 0x10, 0x12, 0x0C, 
												  0x0E, 0x05, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 
												  0x07, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xC4, 0x1C, 0x1D, 0x1E, 0x1F, 0x11, 0x13, 0x0D, 
												  0x0F, 0x04, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 
												  0x06, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xC6, 0x28, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xC7, 0x41, 0x01, 0x0D, 0x11, 0x09, 0x15, 0x19, 
												  0x4F, 0x10, 0xD7, 0xCF, 0x19, 0x1B, 0x1D, 0x03, 
												  0x02, 0x25, 0x30, 0x00, 0x03, 0xFF, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xC8, 0x61, 0x00, 0x31, 0x42, 0x54, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xCA, 0xCB, 0x43);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xCD, 0x0E, 0x64, 0x64, 0x20, 0x1E, 0x6B, 0x06, 
												  0x83);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xD2, 0xE3, 0x2B, 0x38, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xD4, 0x00, 0x01, 0x00, 0x0E, 0x04, 0x44, 0x08, 
												  0x10, 0x00, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xE6, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
												  0xFF);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xE7, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xF0, 0x12, 0x03, 0x20, 0x00, 0xFF);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 	0xF3, 0x00);

	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx); // DCS_Short_Write_NP(0x11);
	mipi_dsi_msleep(&dsi_ctx, 150);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);  // DCS_Short_Write_NP(0x29);
	mipi_dsi_msleep(&dsi_ctx, 20);

	return dsi_ctx.accum_err;
}
#endif

static int boe_th101mb31ig002_disable(struct drm_panel *panel) {
	struct boe_th101mb31ig002 *ctx = container_of(panel,
						      struct boe_th101mb31ig002,
						      panel);
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	if (ctx->desc->backlight_off_to_display_off_delay_ms)
		mipi_dsi_msleep(&dsi_ctx, ctx->desc->backlight_off_to_display_off_delay_ms);

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);

	mipi_dsi_msleep(&dsi_ctx, 120);

	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);

	if (ctx->desc->enter_sleep_to_reset_down_delay_ms)
		mipi_dsi_msleep(&dsi_ctx, ctx->desc->enter_sleep_to_reset_down_delay_ms);

	return dsi_ctx.accum_err;
}

static int boe_th101mb31ig002_unprepare(struct drm_panel *panel) {
	struct boe_th101mb31ig002 *ctx = container_of(panel,
							struct boe_th101mb31ig002,
							panel);

	gpiod_set_value_cansleep(ctx->reset, 1);
	// gpiod_set_value_cansleep(ctx->enable, 0);
	regulator_disable(ctx->power);

	if (ctx->desc->power_off_delay_ms)
		msleep(ctx->desc->power_off_delay_ms);

	return 0;
}

static int boe_th101mb31ig002_prepare(struct drm_panel *panel) {
	struct boe_th101mb31ig002 *ctx = container_of(panel,
							struct boe_th101mb31ig002,
							panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_enable(ctx->power);
	if (ret) {
		dev_err(dev, "Failed to enable power supply: %d\n", ret);
		return ret;
	}

	if (ctx->desc->vcioo_to_lp11_delay_ms)
		msleep(ctx->desc->vcioo_to_lp11_delay_ms);

	if (ctx->desc->lp11_before_reset) {
		ret = mipi_dsi_dcs_nop(ctx->dsi);
		if (ret)
			return ret;
	}

	if (ctx->desc->lp11_to_reset_delay_ms)
		msleep(ctx->desc->lp11_to_reset_delay_ms);

	// gpiod_set_value_cansleep(ctx->enable, 1);
	msleep(50);
	boe_th101mb31ig002_reset(ctx);

	ret = ctx->desc->init(ctx);
	if (ret)
		return ret;

	return 0;
}


static const struct drm_display_mode boe_th101mb31ig002_default_mode = {
	.clock		= 73500,
	.hdisplay	= 800,
	.hsync_start	= 800 + 64,
	.hsync_end	= 800 + 64 + 16,
	.htotal		= 800 + 64 + 16 + 64,
	.vdisplay	= 1280,
	.vsync_start	= 1280 + 2,
	.vsync_end	= 1280 + 2 + 4,
	.vtotal		= 1280 + 2 + 4 + 12,
	.width_mm	= 135,
	.height_mm	= 216,
	.type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static const struct panel_desc boe_th101mb31ig002_desc = {
	.modes = &boe_th101mb31ig002_default_mode,
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_EOT_PACKET |
			  MIPI_DSI_MODE_LPM,
	.init = boe_th101mb31ig002_enable,
};

#ifdef IS_ORIGINAL
static const struct drm_display_mode starry_er88577_default_mode = {
	.clock	= (800 + 25 + 25 + 25) * (1280 + 20 + 4 + 12) * 60 / 1000,
	.hdisplay = 800,
	.hsync_start = 800 + 25,
	.hsync_end = 800 + 25 + 25,
	.htotal = 800 + 25 + 25 + 25,
	.vdisplay = 1280,
	.vsync_start = 1280 + 20,
	.vsync_end = 1280 + 20 + 4,
	.vtotal = 1280 + 20 + 4 + 12,
	.width_mm = 135,
	.height_mm = 216,
	.type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static const struct panel_desc starry_er88577_desc = {
	.modes = &starry_er88577_default_mode,
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
		      MIPI_DSI_MODE_LPM,
	.init = starry_er88577_init_cmd,
	.lp11_before_reset = true,
	.vcioo_to_lp11_delay_ms = 5,
	.lp11_to_reset_delay_ms = 50,
	.backlight_off_to_display_off_delay_ms = 100,
	.enter_sleep_to_reset_down_delay_ms = 100,
	.power_off_delay_ms = 1000,
};
#else

#define H_ACTIVE		800
#define H_FRONT_PORCH 	80 		// 80
#define H_SYNC			20 		// 20
#define H_BACK_PORCH	80 		// 80

#define V_ACTIVE		1280
#define V_FRONT_PORCH	10		// 10
#define V_SYNC			4		// 4
#define V_BACK_PORCH	12		// 12

static const struct drm_display_mode starry_er88577_default_mode = {
	.clock	= (H_ACTIVE + H_FRONT_PORCH + H_SYNC + H_BACK_PORCH) * (V_ACTIVE + V_FRONT_PORCH + V_SYNC + V_BACK_PORCH) * 60 / 1000,
	.hdisplay = H_ACTIVE,
	.hsync_start = H_ACTIVE + H_FRONT_PORCH,
	.hsync_end = H_ACTIVE + H_FRONT_PORCH + H_SYNC,
	.htotal = H_ACTIVE + H_FRONT_PORCH + H_SYNC + H_BACK_PORCH,

	.vdisplay = V_ACTIVE,
	.vsync_start = V_ACTIVE + V_FRONT_PORCH,
	.vsync_end = V_ACTIVE + V_FRONT_PORCH + V_SYNC,
	.vtotal = V_ACTIVE + V_FRONT_PORCH + V_SYNC + V_BACK_PORCH,

	.width_mm = 135,
	.height_mm = 216,
	.type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static const struct panel_desc starry_er88577_desc = {
	.modes = &starry_er88577_default_mode,
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
		          MIPI_DSI_MODE_LPM,
	.init = starry_er88577_init_cmd,
	.lp11_before_reset = true,
	.vcioo_to_lp11_delay_ms = 5,
	.lp11_to_reset_delay_ms = 50,
	.backlight_off_to_display_off_delay_ms = 100,
	.enter_sleep_to_reset_down_delay_ms = 100,
	.power_off_delay_ms = 1000,
};
#endif

static int boe_th101mb31ig002_get_modes(struct drm_panel *panel,
					struct drm_connector *connector) {

	struct boe_th101mb31ig002 *ctx = container_of(panel,
						      struct boe_th101mb31ig002,
						      panel);
	const struct drm_display_mode *desc_mode = ctx->desc->modes;
	connector->display_info.bpc = 0;

	/*
	 * TODO: Remove once all drm drivers call
	 * drm_connector_set_orientation_from_panel()
	 */
	drm_connector_set_panel_orientation(connector, ctx->orientation);
	return drm_connector_helper_get_modes_fixed(connector, desc_mode);
}

// static enum drm_panel_orientation
// boe_th101mb31ig002_get_orientation(struct drm_panel *panel)
// {
// 	struct boe_th101mb31ig002 *ctx = container_of(panel,
// 						      struct boe_th101mb31ig002,
// 						      panel);

// 	return ctx->orientation;
// }


static const struct drm_panel_funcs boe_th101mb31ig002_funcs = {
	.prepare = boe_th101mb31ig002_prepare,
	.unprepare = boe_th101mb31ig002_unprepare,
	.disable = boe_th101mb31ig002_disable,
	.get_modes = boe_th101mb31ig002_get_modes,
	// .get_orientation = boe_th101mb31ig002_get_orientation,
};
#endif

static int panel_show_register(struct mipi_dsi_device *dsi, uint8_t cmd, size_t len) {
	uint8_t buf[4] = { 0 };
	int err;
	ssize_t count;
	struct device *dev = &dsi->dev;
	char result_str[16];

	err = mipi_dsi_set_maximum_return_packet_size(dsi, len);
	if (err < 0) {
		DRM_DEV_ERROR(dev, "failed to set maximum return packet_size for cmd 0x%02x, ret = %d\n", cmd, err);
		return err;
	}

	count = mipi_dsi_dcs_read(dsi, cmd, buf, len);
	if (count < 0) {
		DRM_DEV_ERROR(dev, "failed to read cmd 0x%02x, ret = %ld\n", cmd, count);
		return count;
	}
	snprintf(result_str, sizeof(result_str), "%02x %02x %02x %02x", buf[0], buf[1], buf[2], buf[3]);
	if ((count > 0) && (count < 4))
		result_str[3*count - 1] = 0;

	DRM_DEV_INFO(dev, "cmd 0x%02x returned %ld bytes: %s\n", cmd, count, result_str);
	return 0;
}

static void panel_show_registers(struct mipi_dsi_device *dsi, bool hs) {
	static const uint8_t read_commands[] = { 0x05, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
										     0x52, 0x54, 0x56, 0xda, 0xdb, 0xdc, 0x04,
										     0x09, 0x45, 0x0a };
	static const uint8_t read_len[] =      {    1,    1,    1,    1,    1,    1,    1,
												1,    1,    1,    1,    1,    1,    3,
												4,    2,    1 };
	struct device *dev = &dsi->dev;
	int i;
	unsigned long prev_flags = dsi->mode_flags;
	if (hs)
		dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	else
		dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	DRM_DEV_INFO(dev, "Reading registers in %s mode\n", dsi->mode_flags & MIPI_DSI_MODE_LPM  ? "LP" : "HS");
	for (i = 0; i < ARRAY_SIZE(read_commands); i++) {
		panel_show_register(dsi, read_commands[i], read_len[i]);
	}

	dsi->mode_flags = prev_flags;
}

static int enable_bsit(struct mipi_dsi_device *dsi, bool hs) {
	struct device *dev = &dsi->dev;
	char bsit_cmd[] = { 0xB1, 0x11, 0x01, 0x47, 0xFF };
	int err;
	unsigned long prev_flags = dsi->mode_flags;
	if (hs)
		dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	else
		dsi->mode_flags |= MIPI_DSI_MODE_LPM;
	
	DRM_DEV_INFO(dev, "Enable BSIT in %s mode\n", dsi->mode_flags & MIPI_DSI_MODE_LPM  ? "LP" : "HS");
	err = mipi_dsi_dcs_write_buffer(dsi, bsit_cmd, sizeof(bsit_cmd));
	if (err < 0) {
		DRM_DEV_ERROR(dev, "failed to write BSIT CMD, err = %d\n", err);
	}

	dsi->mode_flags = prev_flags;
	return err;
}

static int exit_sleep(struct drm_panel *panel) {
	struct er88577b *ctx = container_of(panel, struct er88577b, panel);
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = panel->dev;
	int err;

	DRM_DEV_INFO(dev, "Exiting sleep mode\n");
	err = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (err < 0) {
		DRM_DEV_ERROR(dev, "failed to exit sleep mode ret = %d\n", err);
		return err;
	}
	msleep(150);

	DRM_DEV_INFO(dev, "Set display ON\n");
	err = mipi_dsi_dcs_set_display_on(dsi);
	if (err < 0) {
		DRM_DEV_ERROR(dev, "failed to set display on ret = %d\n", err);
		return err;
	}

	msleep(150);
	if (ctx->debug) {
		panel_show_registers(dsi, false);
		panel_show_registers(dsi, true);		
	}

	return 0;
}

static int er88577b_enable(struct drm_panel *panel) {
	// struct er88577b *ctx = container_of(panel, struct er88577b, panel);
	// struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi, .accum_err = 0 };

	DRM_DEV_INFO(panel->dev, "Panel Enable\n");

	// TODO Enable build-in self test
	// TODO show registers
	return 0;
}

static int er88577b_disable(struct drm_panel *panel) {
	struct er88577b *ctx = container_of(panel, struct er88577b, panel);
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi, .accum_err = 0 };

	DRM_DEV_INFO(panel->dev, "Panel Disable\n");

	// if (ctx->desc->backlight_off_to_display_off_delay_ms)
	// 	mipi_dsi_msleep(&dsi_ctx, ctx->desc->backlight_off_to_display_off_delay_ms);
	
	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);

	mipi_dsi_msleep(&dsi_ctx, 120);

	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);

	// if (ctx->desc->enter_sleep_to_reset_down_delay_ms)
	// 	mipi_dsi_msleep(&dsi_ctx, ctx->desc->enter_sleep_to_reset_down_delay_ms);

	return dsi_ctx.accum_err;
}

static int er88577b_prepare(struct drm_panel *panel) {
	struct er88577b *ctx = container_of(panel, struct er88577b, panel);
	const struct er88577b_panel_desc *desc = ctx->desc;
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = panel->dev;
	int i;
	int err;

	DRM_DEV_INFO(dev, "Panel Prepare\n");
	DRM_DEV_INFO(dev, "Togglint reset\n");
	
	mdelay(150);
	gpiod_set_value_cansleep(ctx->reset, 0); // 0 - включение панели
	mdelay(150);

	for (i = 0; i < desc->num_init_cmds; i++) {
		const struct er88577b_init_cmd *cmd = &desc->init_cmds[i];
		switch (cmd->type) {
		case CMD_TYPE_DELAY:
			DRM_DEV_INFO(dev, "Waiting %d ms\n", cmd->data[0]);
			msleep(cmd->data[0]);
			err = 0;
			break;
		
		case CMD_TYPE_DCS:
			DRM_DEV_INFO(dev, "Writting command starting with 0x%02x\n", cmd->data[0]);
			err = mipi_dsi_dcs_write_buffer(dsi, cmd->data, cmd->len);
			break;

		default:
			err = -EINVAL;
		}

		if (err < 0) {
			DRM_DEV_ERROR(dev, "failed to write CMD#0x%x, err=%d\n", cmd->data[0], err);
			return err;
		}
	}

	if (ctx->bsit) {
		enable_bsit(dsi, false);
	}
	return exit_sleep(panel);
}

static int er88577b_unprepare(struct drm_panel *panel) {
	struct er88577b *ctx = container_of(panel, struct er88577b, panel);

	DRM_DEV_INFO(panel->dev, "Panel Unprepare, asserting reset\n");
	gpiod_set_value_cansleep(ctx->reset, 1);
	regulator_disable(ctx->power);

	// if (ctx->desc->power_off_delay_ms)
	// 	msleep(ctx->desc->power_off_delay_ms);

	return 0;
}

static int er88577b_get_modes(struct drm_panel *panel,
			    struct drm_connector *connector) {

	struct er88577b *ctx = container_of(panel,
						      struct er88577b,
						      panel);
	const struct drm_display_mode *desc_mode = &ctx->desc->mode;
	// struct drm_display_mode *mode;

	DRM_DEV_INFO(panel->dev, "Panel Get Modes\n");

	connector->display_info.bpc = 0;
	drm_connector_set_panel_orientation(connector, ctx->orientation);
	return drm_connector_helper_get_modes_fixed(connector, desc_mode);
}

static const struct drm_panel_funcs er88577b_funcs = {
	.enable = er88577b_enable, // Not OK
	.disable = er88577b_disable, // Ok
	.prepare = er88577b_prepare, // Not OK
	.unprepare = er88577b_unprepare, // OK
	.get_modes = er88577b_get_modes, // OK
};

static const struct er88577b_init_cmd eqt700hky008p_init_cmds[] = {
	_INIT_CMD_DCS(0xE0,0xAB,0xBA),
	_INIT_CMD_DCS(0xE1,0xBA,0xAB),
	_INIT_CMD_DCS(0xB1,0x10,0x01,0x47,0xFF),
	_INIT_CMD_DCS(0xB2,0x0C,0x14,0x04,0x50,0x50,0x14),
	_INIT_CMD_DCS(0xB3,0x56,0xD3,0x00),
	_INIT_CMD_DCS(0xB4,0x22,0x30,0x04),
	_INIT_CMD_DCS(0xB6,0xB0,0x00,0x00,0x10,0x00,0x10,0x00),
	_INIT_CMD_DCS(0xB7,0x0E,0x00,0xFF,0x08,0x08,0xFF,0xFF,0x00),
	_INIT_CMD_DCS(0xB8,0x05,0x12,0x29,0x49,0x48),
	_INIT_CMD_DCS(0xB9,0x7F,0x69,0x57,0x4C,0x47,0x37,0x3C,0x25,0x3E,0x3C,0x3B,0x58,0x45,0x4D,0x40,0x3F,0x35,0x27,0x06,0x7F,0x69,0x57,0x4C,0x47,0x37,0x3C,0x25,0x3E,0x3C,0x3B,0x58,0x45,0x4D,0x40,0x3F,0x35,0x27,0x06),
	_INIT_CMD_DCS(0xC0,0x98,0x76,0x12,0x34,0x33,0x33,0x44,0x44,0x06,0x04,0x8A,0x04,0x0F,0x00,0x00,0x00),
	_INIT_CMD_DCS(0xC1,0x53,0x94,0x02,0x85,0x06,0x04,0x8A,0x04,0x54,0x00),
	_INIT_CMD_DCS(0xC2,0x37,0x09,0x08,0x89,0x08,0x11,0x22,0x21,0x44,0xBB,0x18,0x00),
	_INIT_CMD_DCS(0xC3,0x9C,0x1D,0x1E,0x1F,0x10,0x12,0x0C,0x0E,0x05,0x24,0x24,0x24,0x24,0x24,0x24,0x07,0x24,0x24,0x24,0x24,0x24,0x24),
	_INIT_CMD_DCS(0xC4,0x1C,0x1D,0x1E,0x1F,0x11,0x13,0x0D,0x0F,0x04,0x24,0x24,0x24,0x24,0x24,0x24,0x06,0x24,0x24,0x24,0x24,0x24,0x24),
	_INIT_CMD_DCS(0xC6,0x28,0x28),
	_INIT_CMD_DCS(0xC7,0x41,0x01,0x0D,0x11,0x09,0x15,0x19,0x4F,0x10,0xD7,0xCF,0x19,0x1B,0x1D,0x03,0x02,0x25,0x30,0x00,0x03,0xFF,0x00),
	_INIT_CMD_DCS(0xC8,0x61,0x00,0x31,0x42,0x54,0x16),
	_INIT_CMD_DCS(0xCA,0xCB,0x43),
	_INIT_CMD_DCS(0xCD,0x0E,0x64,0x64,0x20,0x1E,0x6B,0x06,0x83),
	_INIT_CMD_DCS(0xD2,0xE3,0x2B,0x38,0x00),
	_INIT_CMD_DCS(0xD4,0x00,0x01,0x00,0x0E,0x04,0x44,0x08,0x10,0x00,0x07,0x00),
	_INIT_CMD_DCS(0xE6,0x00,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF),
	_INIT_CMD_DCS(0xE7,0x00,0x00,0x00),
	_INIT_CMD_DCS(0xF0,0x12,0x03,0x20,0x00,0xFF),
	_INIT_CMD_DCS(0xF3,0x00),
	//_INIT_CMD_DCS(0x35,0x00),
	//_INIT_CMD_DCS(0x36,0x00),
};

static const struct er88577b_panel_desc easy_quick_er88577_desc = {
	.mode = {
//		.clock		= 77000,
		.clock		= 83333, //in kHz, actual value is 250MHz/3

/* * The horizontal and vertical timings are defined per the following diagram.
 *
 *               Active                 Front           Sync           Back
 *              Region                 Porch                          Porch
 *     <-----------------------><----------------><-------------><-------------->
 *       //////////////////////|
 *      ////////////////////// |
 *     //////////////////////  |..................               ................
 *                                                _______________
 *     <----- [hv]display ----->
 *     <------------- [hv]sync_start ------------>
 *     <--------------------- [hv]sync_end --------------------->
 *     <-------------------------------- [hv]total ----------------------------->*/
 
		.hdisplay	= 800,
//		.hsync_start	= 800 + 80,
//		.hsync_end	= 800 + 80 + 20,
//		.htotal		= 800 + 80 + 20 + 80,
		.hsync_start	= 800 + 155, //HFP adjusted for 83,333MHz 
		.hsync_end	= 800 + 155 + 20,
		.htotal		= 800 + 155 + 20 + 80,

		.vdisplay	= 1280,
		.vsync_start	= 1280 + 20,
		.vsync_end	= 1280 + 20 + 4,
		.vtotal		= 1280 + 20 + 4 + 12,

		.width_mm	= 94,  // TODO 
		.height_mm	= 151, // TODO
		.type		= DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
	},
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_LPM | MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST,
	.init_cmds = eqt700hky008p_init_cmds,
	.num_init_cmds = ARRAY_SIZE(eqt700hky008p_init_cmds),
};

static int er88577b_dsi_probe(struct mipi_dsi_device *dsi) {
	struct er88577b *ctx;
	const struct er88577b_panel_desc *desc;

	struct device *dev = &dsi->dev;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	mipi_dsi_set_drvdata(dsi, ctx);
	ctx->dsi = dsi;
	desc = of_device_get_match_data(dev);

	dsi->lanes = desc->lanes;
	dsi->format = desc->format;
	dsi->mode_flags = desc->mode_flags;
	ctx->desc = desc;
	dev_warn(dev, "Using %d lanes\n", dsi->lanes);

	ctx->bsit = of_property_read_bool(dsi->dev.of_node, "bsit");
	if (ctx->bsit) {
		DRM_DEV_INFO(dev, "Enable Build in self test\n");
	}
	
	ctx->debug = of_property_read_bool(dsi->dev.of_node, "debug");
	ctx->power = devm_regulator_get(dev, "power");
	if (IS_ERR(ctx->power))
		return dev_err_probe(dev, PTR_ERR(ctx->power),
				     "Failed to get power regulator\n");

	ctx->reset = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset))
		return dev_err_probe(dev, PTR_ERR(ctx->reset),
				     "Failed to get reset GPIO\n");

	ret = of_drm_get_panel_orientation(dsi->dev.of_node,
					   &ctx->orientation);
	if (ret)
		return dev_err_probe(dev, ret,
				     "Failed to get orientation\n");

	drm_panel_init(&ctx->panel, dev, &er88577b_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return ret;

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err_probe(dev, ret,
			      "Failed to attach panel to DSI host\n");
		drm_panel_remove(&ctx->panel);
		return ret;
	}

	return 0;
}


static int er88577b_dsi_remove(struct mipi_dsi_device *dsi) {
	struct er88577b *ctx = mipi_dsi_get_drvdata(dsi);
	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);
	return 0;
}

static const struct of_device_id easy_quick_er88577b_of_match[] = {
	{
		.compatible = "easy_quick,er88577b",
		.data = &easy_quick_er88577_desc,
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, easy_quick_er88577b_of_match);

static struct mipi_dsi_driver er88577b_mipi_driver = {
	.driver = {
		.name = DSI_DRIVER_NAME,
		.of_match_table = easy_quick_er88577b_of_match,
	},

	.probe = er88577b_dsi_probe,
	.remove = er88577b_dsi_remove
};
module_mipi_dsi_driver(er88577b_mipi_driver);


MODULE_AUTHOR("Ivan Maximov <gluckmaker@gmail.com>");
MODULE_AUTHOR("Vladimir Meshkov <glavmonter@gmail.com>");
MODULE_DESCRIPTION("er88577b-based Easy Quick EQT700HKY008P WUXGA DSI panel");
MODULE_LICENSE("GPL");
