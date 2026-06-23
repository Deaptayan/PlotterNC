#ifndef PLOTTER_DRV_DISPLAY_H
#define PLOTTER_DRV_DISPLAY_H

void drv_display_init();
void drv_display_clear();
void drv_display_set_status(const char *line1, const char *line2);
void drv_display_update();

#endif
