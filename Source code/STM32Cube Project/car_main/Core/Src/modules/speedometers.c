#include "modules/speedometers.h"
#include <math.h>


//void init_opt_rotation_speed_meter(OptSpeedometer* meter) { //init
//
//    meter->last_capture = 0;
//    meter->last_pulse_tick = 0;
//    meter->frequency_filtered = 0.0f;
//    meter->freq_idx = 0;
//    meter->freq_count = 0;
//
//    for(int i = 0; i < AVG_SAMPLES; i++) {
//        meter->freq_buf[i] = 0.0f;
//    }
//}
//
//void handle_opt_capture(OptSpeedometer* meter, uint32_t capture_value, uint32_t current_tick) {
//    if (meter == NULL) return;
//
//    uint32_t diff;
//
//    if (capture_value >= meter->last_capture) {
//        diff = capture_value - meter->last_capture;
//    } else {
//        diff = (0xFFFF - meter->last_capture) + capture_value + 1;
//    }
//
//    meter->last_capture = capture_value;
//
//    if (diff == 0) return;
//
//    float hz = TIM_FREQ / (float)diff;
//
//    // Calculating the current average
//    float cur_avg = 0.0f;
//    if (meter->freq_count > 0) {
//        for (uint8_t i = 0; i < meter->freq_count; ++i) {
//            cur_avg += meter->freq_buf[i];
//        }
//        cur_avg /= (float)meter->freq_count;
//    }
//
//    // Checking the maximum frequency
//    if (hz > MAX_ACCEPT_HZ) {
//        return;
//    }
//
//    if (meter->freq_count > 0 && cur_avg > 0.0f) {
//        float ratio = (hz > cur_avg) ? (hz / cur_avg) : (cur_avg / hz);
//        if (ratio > MAX_REL_CHANGE) {
//            return;
//        }
//    }
//
//    // Adding to buffer
//    meter->freq_buf[meter->freq_idx] = hz;
//    meter->freq_idx = (meter->freq_idx + 1) % AVG_SAMPLES;
//    if (meter->freq_count < AVG_SAMPLES) {
//        meter->freq_count++;
//    }
//
//    // Calculating the filtered frequency
//    float sum = 0.0f;
//    for (uint8_t i = 0; i < meter->freq_count; ++i) {
//        sum += meter->freq_buf[i];
//    }
//    meter->frequency_filtered = sum / (float)meter->freq_count;
//
//    meter->last_pulse_tick = current_tick;
//}
//
//float get_opt_frequency(OptSpeedometer* meter, uint32_t current_tick) {
//    if (meter == NULL) return 0.0f;
//
//    if ((current_tick - meter->last_pulse_tick) > TIMEOUT_MS) {
//        meter->frequency_filtered = 0.0f;
//        meter->freq_idx = 0;
//        meter->freq_count = 0;
//        for(int i = 0; i < AVG_SAMPLES; i++) {
//            meter->freq_buf[i] = 0.0f;
//        }
//    }
//
//    return meter->frequency_filtered;
//}
//
//float get_opt_speed(OptSpeedometer* meter, uint32_t current_tick) {
//    float frequency = get_opt_frequency(meter, current_tick);
//    float wheel_circumference = PI * WHEEL_DIAMETER;
//    float speed = wheel_circumference * frequency;
//    return speed;
//}
