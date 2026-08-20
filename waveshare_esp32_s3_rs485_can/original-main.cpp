/**
 * @file        main.cpp
 * @brief       Entry point and main program for Modbus TCP protocol converter
 * @author      Copyright (C) Peter Ivanov, 2026
 *
 * Created      2026-05-04 11:30:53
 * Last modify: 2026-06-10 20:37:22 ivanovp {Time-stamp}
 * Licence:     GPL
 */

#include <Arduino.h>
#include "HardwareSerial.h"
#include <WiFi.h>

// Modbus TCP-to-RTU bridge include
#include "ModbusBridgeWiFi.h"
// Modbus RTU client include
#include "ModbusClientRTU.h"

#define BOARD_WEMOS_S2_MINI                 1   // Do not modify!
#define BOARD_ESP32DEV                      2   // Do not modify!
#define BOARD_WAVESHARE_ESP32_S3_RS485_CAN  3   // Do not modify!

#ifndef BOARD
/* Select board: BOARD_WEMOS_S2_MINI or BOARD_ESP32DEV */
#define BOARD                               BOARD_ESP32DEV
#endif

#if BOARD == BOARD_WAVESHARE_ESP32_S3_RS485_CAN
// Waveshare ESP32-S3-RS485-CAN
#define RS485_RX_PIN                        18
#define RS485_TX_PIN                        17
#define RS485_DE_nRE_PIN                    21
#define RS485_SERIAL_PORT                   Serial1
#define LED                                 -1
#elif BOARD == BOARD_WEMOS_S2_MINI
// Wemos S2 Mini
#define RS485_RX_PIN                        18
#define RS485_TX_PIN                        17
#define RS485_DE_nRE_PIN                    16
#define RS485_SERIAL_PORT                   Serial1
#define LED                                 LED_BUILTIN
#else
// ESP32DEV, ESP32-WROOM-32D-NodeMCU, ESP32-DEVKITC-32D
#define RS485_RX_PIN                        16
#define RS485_TX_PIN                        17
#define RS485_DE_nRE_PIN                    15
#define RS485_SERIAL_PORT                   Serial2
#define LED                                 2
#endif
#define RS485_BAUDRATE                      9600
#define RS485_SERIAL_MODE                   SERIAL_8N1

/* Modbus configuration */
#define MODBUS_SLAVE_ID                     1
#define MODBUS_BRIDGE_TCP_PORT              502

#define MODBUS_BRIDGE_MAX_CLIENT_NUM        4
#define MODBUS_BRIDGE_INACTIVITY_TIMEOUT_MS 5000

#ifndef SSID
#define SSID "My SSID"
#endif
#ifndef PASSWORD
#define PASSWORD "My password"
#endif

// RTU client that communicates with the RS485 Modbus slave.
ModbusClientRTU modbus_client(RS485_DE_nRE_PIN);

// WiFi bridge that forwards Modbus TCP requests to the RTU client.
ModbusBridgeWiFi modbus_bridge;

// Timestamp used to rate-limit status messages on the serial monitor.
uint32_t print_status_timestamp = 0;

/*
 * Toggle the LED state.
 */
void led_toggle()
{
#if LED >= 0
    digitalWrite(LED, digitalRead(LED) == LOW ? HIGH : LOW);
#endif
}

/*
 * Switch the LED off.
 */
void led_off()
{
#if LED >= 0
    digitalWrite(LED, LOW);
#endif
}

/*
 * Switch the LED on.
 */
void led_on()
{
#if LED >= 0
    digitalWrite(LED, HIGH);
#endif
}

const char* wifi_status_to_string(wl_status_t wifi_status)
{
    switch (wifi_status)
    {
        case WL_NO_SHIELD:
            return "NO_SHIELD";
        case WL_IDLE_STATUS:
            return "IDLE_STATUS";
        case WL_NO_SSID_AVAIL:
            return "NO_SSID_AVAIL";
        case WL_SCAN_COMPLETED:
            return "SCAN_COMPLETED";
        case WL_CONNECTED:
            return "CONNECTED";
        case WL_CONNECT_FAILED:
            return "CONNECT_FAILED";
        case WL_CONNECTION_LOST:
            return "CONNECTION_LOST";
        case WL_DISCONNECTED:
            return "DISCONNECTED";
        default:
            return "UNKNOWN";
    }
}

/*
 * Reset the board if WiFi drops after startup.
 */
void reset_board_on_wifi_disconnect(wl_status_t wifi_status)
{
    WiFi.disconnect(true);
    Serial.printf("WiFi disconnected, status: %s (%d)\r\n", wifi_status_to_string(wifi_status), static_cast<int>(wifi_status));
    Serial.println("Waiting 2 seconds before restarting...\r\n");
    Serial.flush();
    delay(2000);
    ESP.restart();
}

/*
 * Initialize hardware.
 * Connect to WiFi AP.
 * Setup Modbus bridge.
 */
void setup()
{
#if LED >= 0
    pinMode(LED, OUTPUT);
    led_on();
#endif

    // Initialize the serial monitor.
    Serial.begin(115200);
    delay(500);
    led_off();
#if BOARD == BOARD_ESP32DEV
    while (!Serial)
    {
    }
#endif
    Serial.println("Modbus TCP bridge started");

    // Initialize the serial port connected to the Modbus RTU bus.
    RTUutils::prepareHardwareSerial(RS485_SERIAL_PORT);
    RS485_SERIAL_PORT.begin(RS485_BAUDRATE, RS485_SERIAL_MODE, RS485_RX_PIN, RS485_TX_PIN);

    // Connect to the configured WiFi network.
    WiFi.setHostname("modbus-bridge-test");
    WiFi.setAutoReconnect(true);
    WiFi.begin(SSID, PASSWORD);
    delay(250);
    const uint32_t delay_granularity_ms = 200;
    const uint32_t timeout_sec = 20;
    uint32_t timeout_cntr = timeout_sec * 1000 / delay_granularity_ms;
    Serial.printf("Waiting up to %d seconds for WiFi connection\r\n", timeout_sec);
    while (!WiFi.isConnected() && timeout_cntr--)
    {
        if (timeout_cntr % (1000 / delay_granularity_ms) == 0)
        {
            /* Print dot in every second */
            Serial.print('.');
        }
        /* Fast blinking when not connected */
        led_toggle();
        delay(delay_granularity_ms);
    }
    Serial.println();
    if (WiFi.isConnected())
    {
        Serial.println("WiFi connected");
    }
    else
    {
        Serial.println("WiFi connection timed out");
        reset_board_on_wifi_disconnect(WiFi.status());
    }

    // Keep the LED on once WiFi is connected.
    led_on();

    // Set RTU Modbus message timeout to 2000ms
    modbus_client.setTimeout(2000);
#if BOARD == BOARD_WEMOS_S2_MINI
    modbus_client.begin(RS485_SERIAL_PORT);
#else
    // Start the Modbus RTU background task on core 1.
    modbus_client.begin(RS485_SERIAL_PORT, 1);
#endif

    modbus_bridge.attachServer(MODBUS_SLAVE_ID, MODBUS_SLAVE_ID, ANY_FUNCTION_CODE, &modbus_client);

    // Print all served Modbus mappings to the serial monitor.
    modbus_bridge.listServer();

    // Start the bridge with the configured TCP port, client limit, and inactivity timeout.
    modbus_bridge.start(MODBUS_BRIDGE_TCP_PORT, MODBUS_BRIDGE_MAX_CLIENT_NUM, MODBUS_BRIDGE_INACTIVITY_TIMEOUT_MS);
}

/*
 * Periodically print bridge status and blink the LED according to WiFi state.
 */
void loop()
{
    wl_status_t wifi_status = WiFi.status();

    if (print_status_timestamp == 0 || print_status_timestamp + 10000 <= millis())
    {
        print_status_timestamp = millis();
        IPAddress wIP = WiFi.localIP();
        Serial.printf("Modbus TCP bridge, WiFi status: %s (%d), IP: %u.%u.%u.%u:%d\r\n", wifi_status_to_string(wifi_status), static_cast<int>(wifi_status), wIP[0], wIP[1], wIP[2], wIP[3], MODBUS_BRIDGE_TCP_PORT);
    }
    if (wifi_status == WL_CONNECTED)
    {
        /* Slow blinking when connected */
        led_toggle();
        delay(500);
    }
    else
    {
        reset_board_on_wifi_disconnect(wifi_status);
    }
}
