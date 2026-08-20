import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.core import CORE

CONF_TX_PIN = "tx_pin"
CONF_RX_PIN = "rx_pin"
CONF_FLOW_CONTROL_PIN = "flow_control_pin"
CONF_BAUD_RATE = "baud_rate"
CONF_TCP_PORT = "tcp_port"
CONF_SLAVE_ID = "slave_id"
CONF_MAX_CLIENTS = "max_clients"
CONF_INACTIVITY_TIMEOUT = "inactivity_timeout"
CONF_RESPONSE_TIMEOUT = "response_timeout"
CONF_UART_NUM = "uart_num"

DEPENDENCIES = ["wifi"]

modbus_tcp_rtu_bridge_ns = cg.esphome_ns.namespace("modbus_tcp_rtu_bridge")
ModbusTcpRtuBridge = modbus_tcp_rtu_bridge_ns.class_(
    "ModbusTcpRtuBridge", cg.Component
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ModbusTcpRtuBridge),
        cv.Required(CONF_TX_PIN): cv.int_range(min=0, max=48),
        cv.Required(CONF_RX_PIN): cv.int_range(min=0, max=48),
        cv.Required(CONF_FLOW_CONTROL_PIN): cv.int_range(min=0, max=48),
        cv.Optional(CONF_BAUD_RATE, default=9600): cv.positive_int,
        cv.Optional(CONF_TCP_PORT, default=502): cv.port,
        cv.Optional(CONF_SLAVE_ID, default=1): cv.int_range(min=1, max=247),
        cv.Optional(CONF_MAX_CLIENTS, default=4): cv.int_range(min=1, max=16),
        cv.Optional(CONF_INACTIVITY_TIMEOUT, default="5s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_RESPONSE_TIMEOUT, default="2s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_UART_NUM, default=1): cv.int_range(min=0, max=2),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_tx_pin(config[CONF_TX_PIN]))
    cg.add(var.set_rx_pin(config[CONF_RX_PIN]))
    cg.add(var.set_flow_control_pin(config[CONF_FLOW_CONTROL_PIN]))
    cg.add(var.set_baud_rate(config[CONF_BAUD_RATE]))
    cg.add(var.set_tcp_port(config[CONF_TCP_PORT]))
    cg.add(var.set_slave_id(config[CONF_SLAVE_ID]))
    cg.add(var.set_max_clients(config[CONF_MAX_CLIENTS]))
    cg.add(var.set_inactivity_timeout_ms(config[CONF_INACTIVITY_TIMEOUT].total_milliseconds))
    cg.add(var.set_response_timeout_ms(config[CONF_RESPONSE_TIMEOUT].total_milliseconds))
    cg.add(var.set_uart_num(config[CONF_UART_NUM]))

    # eModbus 1.7.4 is the same API family used by the original working firmware.
    cg.add_library("miq19/eModbus", "1.7.4")

    # ESPHome 2026.2+ disables Arduino libraries selectively. The built-in wifi
    # dependency normally enables this already, but being explicit keeps this
    # external component self-documenting and robust.
    if CORE.is_esp32 and CORE.using_arduino:
        cg.add_library("WiFi", None)
