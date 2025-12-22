#ifndef INC_MODULES_SPEEDOMETERS_H_
#define INC_MODULES_SPEEDOMETERS_H_

#define AVG_SAMPLES        8        // Number of samples for moving average filter
#define TIM_FREQ           8000000.0f  // Timer clock frequency in Hz (8 MHz)
#define MAX_ACCEPT_HZ      400.0f   // Maximum acceptable frequency (Hz)
#define MAX_REL_CHANGE     2.0f     // Maximum relative change ratio for outlier rejection
#define TIMEOUT_MS         500      // Timeout period for no pulses (ms)
#define WHEEL_DIAMETER     0.03f    // Wheel diameter in meters (for speed calculation)
#define PI 3.14159265f              // Pi constant

typedef struct {
    uint32_t last_capture;
    float frequency_filtered;
    float freq_buf[AVG_SAMPLES];
    uint8_t freq_idx;
    uint8_t freq_count;
    uint32_t last_pulse_tick;
} OptSpeedometer;

void init_opt_rotation_speed_meter(OptSpeedometer* meter);
void handle_opt_capture(OptSpeedometer* meter, uint32_t capture_value, uint32_t current_tick);
float get_opt_frequency(OptSpeedometer* meter, uint32_t current_tick);
float get_opt_speed(OptSpeedometer* meter, uint32_t current_tick);

#endif /* INC_MODULES_SPEEDOMETERS_H_ */
