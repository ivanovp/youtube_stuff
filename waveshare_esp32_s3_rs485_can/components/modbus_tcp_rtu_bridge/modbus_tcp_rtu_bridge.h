#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <ModbusBridgeWiFi.h>
#include <ModbusClientRTU.h>

namespace esphome {
namespace modbus_tcp_rtu_bridge {

class ModbusTcpRtuBridge : public Component {
 public:
  void set_tx_pin(uint8_t pin) { this->tx_pin_ = pin; }
  void set_rx_pin(uint8_t pin) { this->rx_pin_ = pin; }
  void set_flow_control_pin(uint8_t pin) { this->flow_control_pin_ = pin; }
  void set_baud_rate(uint32_t baud_rate) { this->baud_rate_ = baud_rate; }
  void set_tcp_port(uint16_t tcp_port) { this->tcp_port_ = tcp_port; }
  void set_slave_id(uint8_t slave_id) { this->slave_id_ = slave_id; }
  void set_max_clients(uint8_t max_clients) { this->max_clients_ = max_clients; }
  void set_inactivity_timeout_ms(uint32_t timeout) { this->inactivity_timeout_ms_ = timeout; }
  void set_response_timeout_ms(uint32_t timeout) { this->response_timeout_ms_ = timeout; }
  void set_uart_num(uint8_t uart_num) { this->uart_num_ = uart_num; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

 protected:
  uint8_t tx_pin_{17};
  uint8_t rx_pin_{18};
  uint8_t flow_control_pin_{21};
  uint8_t uart_num_{1};
  uint32_t baud_rate_{9600};
  uint16_t tcp_port_{502};
  uint8_t slave_id_{1};
  uint8_t max_clients_{4};
  uint32_t inactivity_timeout_ms_{5000};
  uint32_t response_timeout_ms_{2000};

  HardwareSerial *serial_{nullptr};
  ModbusClientRTU *modbus_client_{nullptr};
  ModbusBridgeWiFi *modbus_bridge_{nullptr};
};

}  // namespace modbus_tcp_rtu_bridge
}  // namespace esphome
