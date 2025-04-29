#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/float32_multi_array.h>
#include "encoder_pins.h"

int encoderPins[] = {
    ENCODER_A_3, ENCODER_B_3, 
    ENCODER_A_4, ENCODER_B_4, 
};

volatile int encoder_counts[2] = {0, 0};
volatile unsigned long lastInterruptTime = 0;
volatile int motorRPM[2] = {0, 0};
const int ticksPerRevolution = 4200;

int count1;
int count2;

volatile unsigned long lastEncoderMicros[2] = {0, 0};
volatile unsigned long encoderPeriodMicros[2] = {0, 0};

rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;

rcl_publisher_t encoder_publisher;
rcl_publisher_t vel_publisher;

std_msgs__msg__Float32MultiArray encoder_msg;
std_msgs__msg__Float32MultiArray vel_msg;


rcl_timer_t timer;
rclc_executor_t executor;


#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if (temp_rc != RCL_RET_OK) { error_loop(); }}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if (temp_rc != RCL_RET_OK) { /* Hata durumu yönetilebilir */ }}

void error_loop() {
    while (1) {
        delay(500);
    }
}

void IRAM_ATTR readEncoder(int index, int pinA, int pinB) {
    unsigned long now = micros();
    unsigned long duration = now - lastEncoderMicros[index];
    lastEncoderMicros[index] = now;
    encoder_counts[index] += (GPIO.in & (1 << pinA)) 
                             ? ((GPIO.in & (1 << pinB)) ? 1 : -1)
                             : ((GPIO.in & (1 << pinB)) ? -1 : 1);
    encoderPeriodMicros[index] = duration;
}

void IRAM_ATTR readEncoder1() { readEncoder(0, ENCODER_A_3, ENCODER_B_3); }
void IRAM_ATTR readEncoder2() { readEncoder(1, ENCODER_A_4, ENCODER_B_4); }

void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
    if (timer == NULL) {
        return;
    }

    int count1 = encoder_counts[0];
    int count2 = encoder_counts[1];
    encoder_counts[0] = 0;
    encoder_counts[1] = 0;

    float rpm1 = 0.0, rpm2 = 0.0;

    unsigned long now = micros();

    // Sadece encoder sinyali geldiyse hesapla
    if (count1 != 0 && encoderPeriodMicros[0] > 0) {
        float frequency1 = 1e6 / encoderPeriodMicros[0]; // Hz
        rpm1 = (frequency1 / ticksPerRevolution) * 60.0;
    }

    if (count2 != 0 && encoderPeriodMicros[1] > 0) {
        float frequency2 = 1e6 / encoderPeriodMicros[1]; // Hz
        rpm2 = (frequency2 / ticksPerRevolution) * 60.0;
    }

    encoder_msg.data.data[0] = count1;
    encoder_msg.data.data[1] = count2;
    RCSOFTCHECK(rcl_publish(&encoder_publisher, &encoder_msg, NULL));

    vel_msg.data.data[0] = rpm1;
    vel_msg.data.data[1] = rpm2;
    RCSOFTCHECK(rcl_publish(&vel_publisher, &vel_msg, NULL));
}


void setup() {
    Serial.begin(921600);

    for (int i = 0; i < 2; i++) {
        pinMode(encoderPins[i], INPUT_PULLUP);
    }

    set_microros_serial_transports(Serial);
    delay(100);

    attachInterrupt(ENCODER_A_3, readEncoder1, CHANGE);
    attachInterrupt(ENCODER_A_4, readEncoder2, CHANGE);

    allocator = rcl_get_default_allocator();
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

    RCCHECK(rclc_node_init_default(&node, "encoder_2", "", &support));

    const unsigned int timer_timeout = 10;
    RCCHECK(rclc_timer_init_default(
        &timer,
        &support,
        RCL_MS_TO_NS(timer_timeout),
        timer_callback));

    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_timer(&executor, &timer));


    RCCHECK(rclc_publisher_init_default(
        &encoder_publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32MultiArray),
        "encoder2"));
               

    RCCHECK(rclc_publisher_init_default(
        &vel_publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32MultiArray),
        "velocity2"));
    
    encoder_msg.data.size = 2;  
    encoder_msg.data.capacity = 2;
    encoder_msg.data.data = (float*)malloc(encoder_msg.data.capacity * sizeof(float));


    vel_msg.data.size = 2;  
    vel_msg.data.capacity = 2;
    vel_msg.data.data = (float*)malloc(vel_msg.data.capacity * sizeof(float));

}

void loop() {
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1)));
}
