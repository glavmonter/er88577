#include <linux/kernel.h>    // kernel header
#include <linux/device.h>
#include <drm/drm_device.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_print.h>
#include <drm/drm_print.h>

#include "drm_probe_helper_ex.h"

/**
 * drm_connector_helper_get_modes_fixed - Duplicates a display mode for a connector
 * @connector: the connector
 * @fixed_mode: the display hardware's mode
 *
 * This function duplicates a display modes for a connector. Drivers for hardware
 * that only supports a single fixed mode can use this function in their connector's
 * get_modes helper.
 *
 * Returns:
 * The number of created modes.
 */
int drm_connector_helper_get_modes_fixed(struct drm_connector *connector,
					 const struct drm_display_mode *fixed_mode)
{
	struct drm_device *dev = connector->dev;
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(dev, fixed_mode);
	if (!mode) {
		drm_err(dev, "Failed to duplicate mode " DRM_MODE_FMT "\n",
		 	DRM_MODE_ARG(fixed_mode));
		return 0;
	}

	if (mode->name[0] == '\0')
		drm_mode_set_name(mode);

	mode->type |= DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode);

	if (mode->width_mm)
		connector->display_info.width_mm = mode->width_mm;
	if (mode->height_mm)
		connector->display_info.height_mm = mode->height_mm;

	return 1;
}