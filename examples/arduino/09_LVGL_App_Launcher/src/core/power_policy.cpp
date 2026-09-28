#include "../core/power_policy.h"

PowerPolicy power_policy_compute(const AppSettings &settings, const BatteryInfo &battery) {
  PowerPolicy policy;
  policy.brightness = settings.brightness;
  policy.sleep_timeout_ms = settings_sleep_timeout_ms(settings.sleep_mode);

  // TODO(user): Low Battery Mode policy.
  // Inputs: settings.low_battery_mode, battery.present, battery.percent (-1 = unknown),
  //         battery.charging, battery.vbus_in
  // Adjust policy.brightness and/or policy.sleep_timeout_ms here, e.g. cap brightness
  // and force a shorter auto-sleep when the mode is on and the battery is low.
  (void)battery;

  return policy;
}
