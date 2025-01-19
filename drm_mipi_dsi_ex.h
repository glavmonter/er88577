#ifndef __DRM_MIPI_DSI_EX_H__
#define __DRM_MIPI_DSI_EX_H__


struct mipi_dsi_device;

#define mipi_dsi_msleep(ctx, delay)	\
	do {				\
		if (!(ctx)->accum_err)	\
			msleep(delay);	\
	} while (0)


/**
 * struct mipi_dsi_multi_context - Context to call multiple MIPI DSI funcs in a row
 */
struct mipi_dsi_multi_context {
	/**
	 * @dsi: Pointer to the MIPI DSI device
	 */
	struct mipi_dsi_device *dsi;

	/**
	 * @accum_err: Storage for the accumulated error over the multiple calls
	 *
	 * Init to 0. If a function encounters an error then the error code
	 * will be stored here. If you call a function and this points to a
	 * non-zero value then the function will be a noop. This allows calling
	 * a function many times in a row and just checking the error at the
	 * end to see if any of them failed.
	 */
	int accum_err;
};

void mipi_dsi_dcs_enter_sleep_mode_multi(struct mipi_dsi_multi_context *ctx);
void mipi_dsi_dcs_exit_sleep_mode_multi(struct mipi_dsi_multi_context *ctx);
void mipi_dsi_dcs_set_display_off_multi(struct mipi_dsi_multi_context *ctx);
void mipi_dsi_dcs_set_display_on_multi(struct mipi_dsi_multi_context *ctx);
void mipi_dsi_dcs_write_buffer_multi(struct mipi_dsi_multi_context *ctx,
				     const void *data, size_t len);

/**
 * mipi_dsi_dcs_write_seq_multi - transmit a DCS command with payload
 *
 * This macro will print errors for you and error handling is optimized for
 * callers that call this multiple times in a row.
 *
 * @ctx: Context for multiple DSI transactions
 * @cmd: Command
 * @seq: buffer containing data to be transmitted
 */
#define mipi_dsi_dcs_write_seq_multi(ctx, cmd, seq...)                  \
	do {                                                            \
		static const u8 d[] = { cmd, seq };                     \
		mipi_dsi_dcs_write_buffer_multi(ctx, d, ARRAY_SIZE(d)); \
	} while (0)


#endif // __DRM_MIPI_DSI_EX_H__
