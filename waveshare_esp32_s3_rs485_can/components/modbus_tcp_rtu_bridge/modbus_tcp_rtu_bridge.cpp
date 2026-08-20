#include "modbus_tcp_rtu_bridge.h"

namespace esphome {
namespace modbus_tcp_rtu_bridge {

static const char *const TAG = "modbus_tcp_rtu_bridge";

void ModbusTcpRtuBridge::setup() {
  ESP_LOGI(TAG, "Starting Modbus TCP <-> RTU bridge");

  // HardwareSerial instances 0..2 are available on ESP32-S3 Arduino.
  this->serial_ = new HardwareSerial(this->uart_num_);
  RTUutils::prepareHardwareSerial(*this->serial_);
  this->serial_->begin(this->baud_rate_, SERIAL_8N1, this->rx_pin_, this->tx_pin_);

  this->modbus_client_ = new ModbusClientRTU(this->flow_control_pin_);
  this->modbus_client_->setTimeout(this->response_timeout_ms_);

  // Match the original firmware: run the RTU worker on core 1.
  this->modbus_client_->begin(*this->serial_, 1);

  this->modbus_bridge_ = new ModbusBridgeWiFi();
  this->modbus_bridge_->attachServer(
      this->slave_id_, this->slave_id_, ANY_FUNCTION_CODE, this->modbus_client_);
  this->modbus_bridge_->start(
      this->tcp_port_, this->max_clients_, this->inactivity_timeout_ms_);

  ESP_LOGI(TAG, "Bridge listening on TCP port %u", this->tcp_port_);
  ESP_LOGI(TAG, "RTU: UART%u, %lu 8N1, TX=%u RX=%u DE/~RE=%u, slave=%u",
           this->uart_num_, static_cast<unsigned long>(this->baud_rate_),
           this->tx_pin_, this->rx_pin_, this->flow_control_pin_, this->slave_id_);
}

void ModbusTcpRtuBridge::dump_config() {
  ESP_LOGCONFIG(TAG, "Modbus TCP <-> RTU Bridge:");
  ESP_LOGCONFIG(TAG, "  TCP port: %u", this->tcp_port_);
  ESP_LOGCONFIG(TAG, "  Maximum TCP clients: %u", this->max_clients_);
  ESP_LOGCONFIG(TAG, "  TCP inactivity timeout: %lu ms",
                static_cast<unsigned long>(this->inactivity_timeout_ms_));
  ESP_LOGCONFIG(TAG, "  Slave ID: %u", this->slave_id_);
  ESP_LOGCONFIG(TAG, "  UART: %u", this->uart_num_);
  ESP_LOGCONFIG(TAG, "  Baud rate: %lu", static_cast<unsigned long>(this->baud_rate_));
  ESP_LOGCONFIG(TAG, "  Format: 8N1");
  ESP_LOGCONFIG(TAG, "  TX pin: GPIO%u", this->tx_pin_);
  ESP_LOGCONFIG(TAG, "  RX pin: GPIO%u", this->rx_pin_);
  ESP_LOGCONFIG(TAG, "  DE/~RE pin: GPIO%u", this->flow_control_pin_);
  ESP_LOGCONFIG(TAG, "  RTU response timeout: %lu ms",
                static_cast<unsigned long>(this->response_timeout_ms_));
}

}  // namespace modbus_tcp_rtu_bridge
}  // namespace esphome
