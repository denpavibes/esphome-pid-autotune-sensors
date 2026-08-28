# ESPHome PID Autotune Status

An ESPHome custom component that exposes the PID Climate autotune status as a text sensor in Home Assistant.

## Usage

You don't need to download any files manually. Just add the repository directly to your ESPHome configuration using the `external_components` block.

```yaml
external_components:
  - source: github://denpavibes/esphome-pid-autotune-sensors

climate:
  - platform: pid
    id: my_pid_climate
    # ... rest of your climate config

text_sensor:
  - platform: pid_autotune
    name: "PID Autotune Status"
    climate_id: my_pid_climate
    update_interval: 5s

### States
The sensor will report one of the following states to Home Assistant:
* **Off**: Autotune is not running.
* **Running**: Autotune is currently in progress.
* **Finished**: Autotune has completed successfully.