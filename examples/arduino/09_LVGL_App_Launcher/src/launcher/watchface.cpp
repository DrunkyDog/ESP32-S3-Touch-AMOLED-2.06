#include <time.h>
#include "../board/board.h"
#include "../protocol/net.h"
#include "../core/settings_store.h"
#include "../components/ui_theme.h"
#include "../launcher/watchface.h"

static lv_timer_t *s_timer = nullptr;
static lv_obj_t *s_arc = nullptr;
static lv_obj_t *s_time = nullptr;
static lv_obj_t *s_seconds = nullptr;
static lv_obj_t *s_date = nullptr;
static lv_obj_t *s_place = nullptr;
static lv_obj_t *s_wifi = nullptr;
static lv_obj_t *s_battery = nullptr;

static const char *battery_symbol(int percent, bool charging) {
  if (charging) return LV_SYMBOL_CHARGE;
  if (percent >= 90) return LV_SYMBOL_BATTERY_FULL;
  if (percent >= 60) return LV_SYMBOL_BATTERY_3;
  if (percent >= 35) return LV_SYMBOL_BATTERY_2;
  if (percent >= 10) return LV_SYMBOL_BATTERY_1;
  return LV_SYMBOL_BATTERY_EMPTY;
}

static void watchface_update(lv_timer_t *t) {
  (void)t;
  time_t now = time(nullptr);
  struct tm local;
  localtime_r(&now, &local);

  if (local.tm_year + 1900 < 2024) {
    lv_label_set_text(s_time, "--:--");
    lv_label_set_text(s_seconds, "");
    lv_label_set_text(s_date, "Set time via Wi-Fi");
  } else {
    char buf[40];
    strftime(buf, sizeof(buf), "%H:%M", &local);
    lv_label_set_text(s_time, buf);
    lv_label_set_text_fmt(s_seconds, "%02d", local.tm_sec);
    strftime(buf, sizeof(buf), "%a, %d %b %Y", &local);
    lv_label_set_text(s_date, buf);
  }
  lv_arc_set_value(s_arc, local.tm_sec);

  lv_label_set_text_fmt(s_place, "%s  " LV_SYMBOL_GPS "  %s", g_settings.location, g_settings.tz_name);

  NetState state = net_state();
  lv_obj_set_style_text_color(s_wifi, state == NET_CONNECTED ? g_ui.text : g_ui.muted, 0);
  lv_obj_set_style_text_opa(s_wifi, state == NET_IDLE ? LV_OPA_30 : LV_OPA_COVER, 0);

  BatteryInfo bat = board_battery();
  if (bat.present && bat.percent >= 0) {
    lv_label_set_text_fmt(s_battery, "%d%% %s", bat.percent, battery_symbol(bat.percent, bat.charging));
  } else {
    lv_label_set_text(s_battery, bat.vbus_in ? "USB " LV_SYMBOL_USB : "--");
  }
}

void watchface_create(lv_obj_t *parent) {
  lv_obj_t *status = ui_row(parent);
  lv_obj_set_width(status, lv_pct(100));
  lv_obj_set_style_pad_hor(status, 40, 0);
  lv_obj_set_style_pad_top(status, 20, 0);
  lv_obj_align(status, LV_ALIGN_TOP_MID, 0, 0);
  s_wifi = ui_label(status, LV_SYMBOL_WIFI, &lv_font_montserrat_22, g_ui.text);
  s_battery = ui_label(status, "--", &lv_font_montserrat_22, g_ui.text);

  s_arc = lv_arc_create(parent);
  lv_obj_set_size(s_arc, 300, 300);
  lv_obj_align(s_arc, LV_ALIGN_CENTER, 0, -10);
  lv_arc_set_rotation(s_arc, 270);
  lv_arc_set_bg_angles(s_arc, 0, 360);
  lv_arc_set_range(s_arc, 0, 59);
  lv_obj_remove_style(s_arc, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(s_arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(s_arc, 8, LV_PART_MAIN);
  lv_obj_set_style_arc_width(s_arc, 8, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(s_arc, g_ui.card, LV_PART_MAIN);
  lv_obj_set_style_arc_color(s_arc, g_ui.accent, LV_PART_INDICATOR);

  // No transform_scale here: a scaled label is drawn through an ARGB layer allocated
  // from the 64 KB LVGL heap (lv_conf.h), which does not fit.
  s_time = ui_label(parent, "--:--", &lv_font_montserrat_48, g_ui.text);
  lv_obj_align(s_time, LV_ALIGN_CENTER, 0, -30);

  s_seconds = ui_label(parent, "", &lv_font_montserrat_28, g_ui.accent);
  lv_obj_align(s_seconds, LV_ALIGN_CENTER, 0, 30);

  s_date = ui_label(parent, "", &lv_font_montserrat_22, g_ui.muted);
  lv_obj_align(s_date, LV_ALIGN_CENTER, 0, 175);

  s_place = ui_label(parent, "", &lv_font_montserrat_18, g_ui.muted);
  lv_obj_align(s_place, LV_ALIGN_BOTTOM_MID, 0, -4);

  s_timer = lv_timer_create(watchface_update, 1000, nullptr);
  watchface_update(nullptr);
}

void watchface_destroy(void) {
  if (s_timer != nullptr) {
    lv_timer_delete(s_timer);
    s_timer = nullptr;
  }
  s_arc = s_time = s_seconds = s_date = s_place = s_wifi = s_battery = nullptr;
}
