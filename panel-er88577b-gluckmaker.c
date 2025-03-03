#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_print.h>

#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>

#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_graph.h>
#include <drm/drm_crtc.h>
#include <drm/drm_device.h>
#include <video/display_timing.h>
#include <video/videomode.h>
#include <linux/backlight.h>

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
	const struct er88577b_init_cmd *init_cmds;
	u32 num_init_cmds;
};

struct er88577b {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	const struct er88577b_panel_desc *desc;

	struct gpio_desc *reset; //30
};

static inline struct er88577b *panel_to_er88577b(struct drm_panel *panel)
{
	return container_of(panel, struct er88577b, panel);
}

static int enable_bsit(struct mipi_dsi_device *dsi, bool hs)
{
	struct device *dev = &dsi->dev;
	char bsit_cmd[] = {0xB1,0x11,0x01,0x47,0xFF};
	int err;
	uint32_t prev_flags = dsi->mode_flags;

	if (hs) dsi->mode_flags&=~MIPI_DSI_MODE_LPM;
	else dsi->mode_flags|=MIPI_DSI_MODE_LPM;

	DRM_DEV_INFO(dev, "Enabling BSIT in %s mode\n", dsi->mode_flags&MIPI_DSI_MODE_LPM?"LP":"HS");
	err = mipi_dsi_dcs_write_buffer(dsi, bsit_cmd, sizeof(bsit_cmd));
	if (err < 0) {
		DRM_DEV_ERROR(dev, "failed to write BSIT CMD, err=%d\n", err);
	}

	dsi->mode_flags=prev_flags;

	return err;
}

static int panel_show_reg(struct mipi_dsi_device *dsi, uint8_t cmd, size_t len)
{
	uint8_t buf[4] = {0};
	int err;
	ssize_t count;
	struct device *dev = &dsi->dev;
	char result_str[16];

	err = mipi_dsi_set_maximum_return_packet_size(dsi, len);
	if (err<0) DRM_DEV_ERROR(dev, "failed to set max return packet size for cmd 0x%02x, ret = %d\n", cmd, err);

	count= mipi_dsi_dcs_read(dsi, cmd, buf, len);
	if (count<0) {
		DRM_DEV_ERROR(dev, "failed to read cmd 0x%02x, ret = %ld\n", cmd, count);
		return count;
	}
	sprintf(result_str, "%02x %02x %02x %02x", buf[0], buf[1], buf[2], buf[3]);
	if ((count>0)&&(count<4)) result_str[3*count-1]=0;
	DRM_DEV_INFO(dev, "cmd 0x%02x returned %ld bytes: %s\n", cmd, count, result_str);
	return 0;
}

static void panel_show_regs(struct mipi_dsi_device *dsi, bool hs)
{
	int i;
	static const uint8_t read_commands[]= {0x05, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x52, 0x54, 0x56, 0xda, 0xdb, 0xdc, 0x04, 0x09, 0x45, 0x0a};
	static const uint8_t read_len[]= {1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 3, 4, 2, 1};
	struct device *dev = &dsi->dev;
	uint32_t prev_flags = dsi->mode_flags;

	if (hs) dsi->mode_flags&=~MIPI_DSI_MODE_LPM;
	else dsi->mode_flags|=MIPI_DSI_MODE_LPM;

	DRM_DEV_INFO(dev, "Reading registers in %s mode\n", dsi->mode_flags&MIPI_DSI_MODE_LPM?"LP":"HS");
	for (i=0; i<sizeof(read_commands); i++) panel_show_reg(dsi, read_commands[i], read_len[i]);

	dsi->mode_flags=prev_flags;
}

static int exit_sleep(struct drm_panel *panel)
{
	struct er88577b *er88577b = panel_to_er88577b(panel);
	struct mipi_dsi_device *dsi = er88577b->dsi;
	struct device *dev = &dsi->dev;
	int err;
//	uint8_t param;

	DRM_DEV_INFO(dev, "Exiting sleep mode\n");
	err = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (err < 0) {
		DRM_DEV_ERROR(dev, "failed to exit sleep mode ret = %d\n", err);
		return err;
	}
	msleep(150);

	DRM_DEV_INFO(dev, "Setting display ON\n");
	err =  mipi_dsi_dcs_set_display_on(dsi);
	if (err < 0) {
		DRM_DEV_ERROR(dev, "failed to set display on ret = %d\n", err);
		return err;
	}
	msleep(150);

	show_regs(dsi, false);
	show_regs(dsi, true);

	return 0;
}

static int init_display(struct drm_panel *panel)
{
	struct er88577b *er88577b = panel_to_er88577b(panel);
	const struct er88577b_panel_desc *desc = er88577b->desc;
	struct mipi_dsi_device *dsi = er88577b->dsi;
	struct device *dev = &dsi->dev;
	unsigned int i;
	int err;

	DRM_DEV_INFO(dev, "In init_display\n");
	DRM_DEV_INFO(dev, "Toggling reset\n");

	mdelay(150);
	gpiod_set_value(er88577b->reset, 0); //Active LOW, it actually goes 1 here
	mdelay(200);

	for (i = 0; i < desc->num_init_cmds; i++) {
		const struct er88577b_init_cmd *cmd = &desc->init_cmds[i];

		switch (cmd->type) {
		case CMD_TYPE_DELAY:
			DRM_DEV_INFO(dev, "Waiting %d ms\n", cmd->data[0]);
			msleep(cmd->data[0]);
			err = 0;
			break;
		case CMD_TYPE_DCS:
			/*err = mipi_dsi_dcs_write(dsi, cmd->data[0],
						 cmd->len <= 1 ? NULL : &cmd->data[1],
						 cmd->len - 1);
			*/
			DRM_DEV_INFO(dev, "Writing command starting with 0x%02x\n", cmd->data[0]);
			err = mipi_dsi_dcs_write_buffer(dsi, cmd->data, cmd->len);
			break;
		default:
			err = -EINVAL;
		}

		if (err < 0) {
			DRM_DEV_ERROR(dev, "failed to write CMD#0x%x, err=%d\n", cmd->data[0], err);
			//msleep(120);
			return err;
		}

	}

//	enable_bsit(dsi);
	return exit_sleep(panel);
}

static int er88577b_enable(struct drm_panel *panel)
{
	struct er88577b *er88577b = panel_to_er88577b(panel);
//	const struct er88577b_panel_desc *desc = er88577b->desc;
	struct mipi_dsi_device *dsi = er88577b->dsi;
	struct device *dev = &dsi->dev;
//	unsigned int i;
//	int err;

	DRM_DEV_INFO(dev, "In er88577b_enable\n");

//	enable_bsit(dsi, true);

	show_regs(dsi, false);
	show_regs(dsi, true);

//	DRM_DEV_INFO(dev, "Reading registers in LP mode:\n");
//	show_regs(dsi);

	return 0;
}

static int er88577b_disable(struct drm_panel *panel)
{
	struct device *dev = panel->dev;

	DRM_DEV_INFO(dev, "In er88577b_disable\n");

	return 0;
}

static int er88577b_prepare(struct drm_panel *panel)
{
	struct device *dev = panel->dev;
	unsigned int i;

	DRM_DEV_INFO(dev, "In er88577b_prepare\n");


	return init_display(panel);
}

static int er88577b_unprepare(struct drm_panel *panel)
{
	struct er88577b *er88577b = panel_to_er88577b(panel);
	struct device *dev = panel->dev;

	DRM_DEV_INFO(dev, "Unpreparing - asserting reset\r\n");
	gpiod_set_value(er88577b->reset, 1);
	msleep(120);
	return 0;
}

static int er88577b_get_modes(struct drm_panel *panel,
			    struct drm_connector *connector)
{
	struct er88577b *er88577b = panel_to_er88577b(panel);
	const struct drm_display_mode *desc_mode = &er88577b->desc->mode;
	struct drm_display_mode *mode;

	DRM_DEV_INFO(panel->dev, "In er88577b_get_modes\n");
	mode = drm_mode_duplicate(connector->dev, desc_mode);
	if (!mode) {
		DRM_DEV_ERROR(&er88577b->dsi->dev, "failed to add mode %ux%ux@%u\n",
			      desc_mode->hdisplay, desc_mode->vdisplay,
			      drm_mode_vrefresh(desc_mode));
		return -ENOMEM;
	}

	drm_mode_set_name(mode);
	drm_mode_probed_add(connector, mode);

	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;

	return 1;
}

static const struct drm_panel_funcs er88577b_funcs = {
	.disable = er88577b_disable,
	.unprepare = er88577b_unprepare,
	.prepare = er88577b_prepare,
	.enable = er88577b_enable,
	.get_modes = er88577b_get_modes,
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

//BSIT test pattern:
//_INIT_CMD_DCS(0xB1,0x11,0x01,0x47,0xFF),

//	_INIT_CMD_DELAY(150),
};

static const struct er88577b_panel_desc eqt700hky008p_desc = {
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

		.width_mm	= 94,
		.height_mm	= 151,
		.type		= DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
	},
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.init_cmds = eqt700hky008p_init_cmds,
	.num_init_cmds = ARRAY_SIZE(eqt700hky008p_init_cmds),
};

static int er88577b_dsi_probe(struct mipi_dsi_device *dsi)
{
	struct er88577b *er_panel;
	const struct er88577b_panel_desc *desc;

	struct device *dev = &dsi->dev;
	int ret = 0;
	
	DRM_DEV_INFO(dev, "In er88577b_dsi_probe\n");
	er_panel = devm_kzalloc(dev, sizeof(struct er88577b), GFP_KERNEL);
	if (!er_panel )
		return -ENOMEM;
	desc = of_device_get_match_data(dev);

	mipi_dsi_set_drvdata(dsi, er_panel);

	er_panel->reset = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(er_panel->reset)) {
		DRM_DEV_ERROR(dev, "failed to get our reset GPIO\n");
		goto error;
	}

	DRM_DEV_INFO(dev, "Calling drm_panel_init\n");
	er_panel->panel.prepare_upstream_first = true;
	drm_panel_init(&er_panel->panel, dev, &er88577b_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	drm_panel_add(&er_panel->panel);

	er_panel->desc = desc;

	er_panel->dsi = dsi;

	dsi->mode_flags = MIPI_DSI_MODE_LPM | MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->lanes = 4;
	dsi->channel = 0;

	DRM_DEV_INFO(dev, "Calling mipi_dsi_attach\n");
	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		return ret;
	}

	return 0;

error:
	return -ENODEV;

}

static void er88577b_dsi_remove(struct mipi_dsi_device *dsi)
{
	struct er88577b *er88577b = mipi_dsi_get_drvdata(dsi);

	DRM_DEV_INFO(&dsi->dev, "In er88577b_remove\n");
	mipi_dsi_detach(dsi);
	drm_panel_remove(&er88577b->panel);
}

static const struct of_device_id panel_dt_ids[] = {
	{ .compatible = "easy_quick_eqt700hky008p", .data = &eqt700hky008p_desc},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, panel_dt_ids);

static struct mipi_dsi_driver er88577b_mipi_driver = {
	.driver = {
		.name = DSI_DRIVER_NAME,
		.of_match_table = panel_dt_ids,
	},
	.probe = er88577b_dsi_probe,
	.remove = er88577b_dsi_remove,
};
module_mipi_dsi_driver(er88577b_mipi_driver);

MODULE_AUTHOR("Ivan Maximov <gluckmaker@gmail.com>");
MODULE_DESCRIPTION("er88577b-based Easy Quick EQT700HKY008P WUXGA DSI panel");
MODULE_LICENSE("GPL");

