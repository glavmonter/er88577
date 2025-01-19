#ifndef __DRM_PROBE_HELPER_EX_H__
#define __DRM_PROBE_HELPER_EX_H__

#include <drm/drm_modes.h>

struct drm_connector;
struct drm_crtc;
struct drm_device;
struct drm_modeset_acquire_ctx;

int drm_connector_helper_get_modes_fixed(struct drm_connector *connector,
					 const struct drm_display_mode *fixed_mode);
					 
#endif
