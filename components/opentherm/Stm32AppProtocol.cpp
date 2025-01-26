#include "string.h"

#include "Stm32AppProtocol.h"

namespace esphome {
namespace opentherm {

ProtocolSerializer::ProtocolSerializer() {
  resetToWaitingStartCondition();
}

void ProtocolSerializer::OnNextData(const uint8_t *data, uint8_t count) {
  for (int i = 0; i < count; ++i)
    OnNextByte(data[i]);
}

void ProtocolSerializer::OnNextByte(const uint8_t value) {
  if (m_WaitingStartCondition) {
    if (value == START_BYTE) {
      m_WaitingStartCondition = false;
      m_WaitingFinishCondition = true;
    } else  // skip all non-start bytes
    {
    }
  } else if (m_WaitingFinishCondition) {
    if (value == STOP_BYTE) {
      if (m_OnPacketReceivedHandler)
        m_OnPacketReceivedHandler();

      if (!m_Streaming)
        processData();

      resetToWaitingStartCondition();
    } else {
      if (value >> 4 != 0)  // invalid serialized byte
      {
        if (m_DataCount != 0 || value != START_BYTE)  // skip all start bytes (if no data received)
        {
          resetToWaitingStartCondition();
        } else  // skip all redundant start bytes silently
        {
        }
      } else if (m_DataCount < BUFFER_SIZE) {
        m_Data[m_DataCount] = value;
        m_DataCount++;

        if (m_DataCount == 2) {
          if (m_Streaming) {
            if (m_StreamingHandler)
              m_StreamingHandler(m_Data[0] << 4 | m_Data[1]);

            m_DataCount = 0;
          } else {
            const uint8_t reqId = m_Data[0] << 4 | m_Data[1];
            if (reqId == STREAM_REQ_ID) {
              m_Streaming = true;
              m_DataCount = 0;
            }
          }
        }
      } else  // data buffer overflow
      {
        resetToWaitingStartCondition();
      }
    }
  }
}

void ProtocolSerializer::processData() {
  if (m_DataCount < 8 || m_DataCount % 2 != 0)  // crc16 takes at least 2 bytes, minimum payload 1 bytes, x2 because of serialization
    return;

  m_DeserializedDataCount = 0;
  memset(m_DeserializedData, 0, sizeof(m_DeserializedData));
  for (int i = 2; i < m_DataCount - 4; i += 2)
    m_DeserializedData[m_DeserializedDataCount++] = ((m_Data[i] & 0xF) << 4) | (m_Data[i + 1] & 0xF);

  uint16_t crcExpected = ((m_Data[m_DataCount - 4] & 0xF) << 12) | ((m_Data[m_DataCount - 3] & 0xF) << 8) | ((m_Data[m_DataCount - 2] & 0xF) << 4) | (m_Data[m_DataCount - 1] & 0xF);
  uint16_t crcActual = 0;  // TODO

  // if (crcExpected==crcActual)
  {
    const uint8_t packetId = ((m_Data[0] & 0xF) << 4) | (m_Data[1] & 0xF);

    if (m_Handlers.count(packetId) > 0) {
      auto handlerInfo = m_Handlers[packetId];

      if (m_DeserializedDataCount == handlerInfo.dataSize) {
        handlerInfo.handler(m_DeserializedData);
      } else {
        // printf("wrong data size\n");
      }
    } else {
      // printf("no handler for %d\n", packetId);
    }
  }
}

void ProtocolSerializer::resetToWaitingStartCondition() {
  m_WaitingStartCondition = true;
  m_WaitingFinishCondition = false;
  m_Streaming = false;

  m_DataCount = 0;
  memset(m_Data, 0, BUFFER_SIZE);
}

}  // namespace opentherm
}  // namespace esphome