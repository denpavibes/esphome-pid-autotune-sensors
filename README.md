# ESPHome PID Autotune Status

An ESPHome custom component that exposes the PID Climate autotune status and phase progress as sensors in Home Assistant. Both sensors automatically appear under the Diagnostic category with appropriate icons. It now also includes a dedicated Switch component to seamlessly start and safely abort the tuning process directly from your dashboard.

## Usage

You don't need to download any files manually. Just add the repository directly to your ESPHome configuration using the `external_components` block.

```yaml
external_components:
  - source: github://denpavibes/esphome-pid-autotune-status

climate:
  - platform: pid
    id: my_pid_climate
    # ... rest of your climate config

switch:
  - platform: pid_autotune
    name: "PID Autotune Switch"
    climate_id: my_pid_climate

text_sensor:
  - platform: pid_autotune
    name: "PID Autotune Status"
    climate_id: my_pid_climate
    update_interval: 5s

sensor:
  - platform: pid_autotune
    name: "PID Autotune Phase"
    climate_id: my_pid_climate
    update_interval: 5s
```

### Switch: Autotune Toggle
The switch allows you to control the autotune process interactively:
* **Turning On:** Triggers the climate controller's autotune function.
* **Turning Off:** Immediately aborts an active autotune by destroying the process in memory and returning the climate mode to Off, preventing the background PID from getting stuck. 
* **Auto-Sync:** The switch automatically toggles itself to the Off position when the autotune finishes or fails, keeping your Home Assistant dashboard perfectly synchronized with the hardware state.

### Text Sensor: Status States
The status sensor will report one of the following states to Home Assistant:
* **Off**: Autotune is not running.
* **Running**: Autotune is currently in progress.
* **Finished**: Autotune completed successfully (data was convergent and symmetrical).
* **Failed**: Autotune finished, but failed to reach amplitude convergence or symmetry.

### Numeric Sensor: Phase Counter
Tracks the `phase_count` during the autotuning process. 
* Returns **0** when the autotuner is Off, Finished, or Failed.
* Increments sequentially during the **Running** state as the relay function oscillates.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
