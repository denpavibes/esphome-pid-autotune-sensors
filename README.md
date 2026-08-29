# ESPHome PID Autotune Status

An ESPHome custom component that exposes the PID Climate autotune status and phase progress as sensors in Home Assistant. Both sensors automatically appear under the Diagnostic category with appropriate icons.

## Usage

You don't need to download any files manually. Just add the repository directly to your ESPHome configuration using the `external_components` block.

```yaml
external_components:
  - source: github://denpavibes/esphome-pid-autotune-status

climate:
  - platform: pid
    id: my_pid_climate
    # ... rest of your climate config

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
