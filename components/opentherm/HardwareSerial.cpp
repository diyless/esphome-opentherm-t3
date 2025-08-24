#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"

#include "HardwareSerial.h"

#define ECHO_UART_PORT_NUM (uart_port_t(1))

namespace esphome {
namespace opentherm {

void HardwareSerial::begin(uint32_t baudRate, uint32_t tx, uint32_t rx) {
  uart_config_t uart_config = {
      .baud_rate = (int)baudRate,
      .data_bits = uart_word_length_t::UART_DATA_8_BITS,
      .parity = uart_parity_t::UART_PARITY_DISABLE,
      .stop_bits = uart_stop_bits_t::UART_STOP_BITS_1,
      .flow_ctrl = uart_hw_flowcontrol_t::UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
      .source_clk = uart_sclk_t::UART_SCLK_APB,
  };

  int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
  intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

  ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, rx, tx, -1, -1));
}

void HardwareSerial::send(uint8_t *data, int len)
{
  uart_write_bytes(ECHO_UART_PORT_NUM, data, len);
}

uint8_t HardwareSerial::read() {
  int len = uart_read_bytes(ECHO_UART_PORT_NUM, _data, 1, 0);
  return len == 1 ? _data[0] : 255;
}

bool HardwareSerial::available() {
  size_t size;
  ESP_ERROR_CHECK(uart_get_buffered_data_len(ECHO_UART_PORT_NUM, &size));

  return size > 0;
}

}  // namespace opentherm
}  // namespace esphome