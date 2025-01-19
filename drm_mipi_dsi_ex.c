#include <linux/kernel.h>    // kernel header
#include <linux/device.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_print.h>

#include "drm_mipi_dsi_ex.h"

/**
 * mipi_dsi_dcs_enter_sleep_mode_multi() - send DCS ENTER_SLEEP_MODE  packet
 * @ctx: Context for multiple DSI transactions
 *
 * Like mipi_dsi_dcs_enter_sleep_mode() but deals with errors in a way that
 * makes it convenient to make several calls in a row.
 */
void mipi_dsi_dcs_enter_sleep_mode_multi(struct mipi_dsi_multi_context *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	ssize_t ret;

	if (ctx->accum_err)
		return;
	
	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0) {
		ctx->accum_err = ret;
		dev_err(dev, "sending DCS ENTER_SLEEP_MODE failed: %d\n", ctx->accum_err);
	}
}

/**
 * mipi_dsi_dcs_exit_sleep_mode_multi() - send DCS EXIT_SLEEP_MODE packet
 * @ctx: Context for multiple DSI transactions
 *
 * Like mipi_dsi_dcs_exit_sleep_mode() but deals with errors in a way that
 * makes it convenient to make several calls in a row.
 */
void mipi_dsi_dcs_exit_sleep_mode_multi(struct mipi_dsi_multi_context *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	ssize_t ret;

	if (ctx->accum_err)
		return;
	
	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		ctx->accum_err = ret;
		dev_err(dev, "sending DCS EXIT_SLEEP_MODE failed: %d\n", ctx->accum_err);
	}
}

/**
 * mipi_dsi_dcs_set_display_off_multi() - send DCS SET_DISPLAY_OFF packet
 * @ctx: Context for multiple DSI transactions
 *
 * Like mipi_dsi_dcs_set_display_off() but deals with errors in a way that
 * makes it convenient to make several calls in a row.
 */
void mipi_dsi_dcs_set_display_off_multi(struct mipi_dsi_multi_context *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	ssize_t ret;

	if (ctx->accum_err)
		return;
	
	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0) {
		ctx->accum_err = ret;
		dev_err(dev, "sending DCS SET_DISPLAY_OFF failed: %d\n", ctx->accum_err);
	}
}

/**
 * mipi_dsi_dcs_set_display_on_multi() - send DCS SET_DISPLAY_ON packet
 * @ctx: Context for multiple DSI transactions
 *
 * Like mipi_dsi_dcs_set_display_on() but deals with errors in a way that
 * makes it convenient to make several calls in a row.
 */
void mipi_dsi_dcs_set_display_on_multi(struct mipi_dsi_multi_context *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	ssize_t ret;

	if (ctx->accum_err < 0)
		return;
	
	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		ctx->accum_err = ret;
		dev_err(dev, "sending DCS SET_DISPLAY_ON failed: %d\n", ctx->accum_err);
	}
}

/**
 * mipi_dsi_dcs_write_buffer_multi - mipi_dsi_dcs_write_buffer_chatty() w/ accum_err
 * @ctx: Context for multiple DSI transactions
 * @data: buffer containing data to be transmitted
 * @len: size of transmission buffer
 *
 * Like mipi_dsi_dcs_write_buffer_chatty() but deals with errors in a way that
 * makes it convenient to make several calls in a row.
 */
void mipi_dsi_dcs_write_buffer_multi(struct mipi_dsi_multi_context *ctx,
				     const void *data, size_t len)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	ssize_t ret;

	if (ctx->accum_err)
		return;
	
	ret = mipi_dsi_dcs_write_buffer(dsi, data, len);
	if (ret < 0) {
		ctx->accum_err = ret;
		dev_err(dev, "sending dcs data %*ph failed: %d\n", (int)len, data, ctx->accum_err);
	}
}