#pragma once

#include "functional"
#include "inttypes.h"
#include "lambdatraits.h"
#include <map>

namespace esphome {
namespace opentherm {

struct CpuStatusRequest;
struct CpuStatusResponse;

class ProtocolSerializer {
public:
  ProtocolSerializer();

  static const int BUFFER_SIZE = 64;
  static const int BAUD_RATE = 200000;
  static const int START_BYTE = 0b10100000;
  static const int STOP_BYTE = 0b01100000;
  static const int STREAM_REQ_ID = 100;

  bool isStreaming() const {
    return m_Streaming;
  };

  void OnNextByte(const uint8_t value);
  void OnNextData(const uint8_t *data, uint8_t count);

  void OnStreaming(std::function<void(const uint8_t)> streamingHandler) {
    m_StreamingHandler = streamingHandler;
  };

  template <typename T>  // clang newline
  int prepareRequest(const T *request, uint8_t *pBuffer) {
    int bufIdx = 0;
    pBuffer[bufIdx++] = START_BYTE;

    constexpr uint8_t id = request->type_id;
    pBuffer[bufIdx++] = id >> 4;
    pBuffer[bufIdx++] = id & 0xF;

    const uint8_t *pRequestData = (const uint8_t *)request;

    for (int i = 0; i < sizeof(T); ++i) {
      pBuffer[bufIdx++] = pRequestData[i] >> 4;
      pBuffer[bufIdx++] = pRequestData[i] & 0xF;
    }

    pBuffer[bufIdx++] = 0;  // crc16
    pBuffer[bufIdx++] = 0;  // crc16
    pBuffer[bufIdx++] = 0;  // crc16
    pBuffer[bufIdx++] = 0;  // crc16
    pBuffer[bufIdx++] = STOP_BYTE;

    return bufIdx;
  };

  template <typename T>  // clang newline
  void On(void (*handler)(const T *)) {
    constexpr uint8_t id = T::type_id;
    m_Handlers[id] = RequestTypeInfo{
        .id = id,                                   // clang newline
        .dataSize = sizeof(T),                      // clang newline
        .handler = (void (*)(const void *))handler  // clang newline
    };
  };

  template <typename T, typename U>  // clang newline
  void On(U &&lambd) {
    constexpr uint8_t id = T::type_id;
    void (*handler)(const T *) = cify(std::forward<U>(lambd));

    m_Handlers[id] = RequestTypeInfo{
        .id = id,                                   // clang newline
        .dataSize = sizeof(T),                      // clang newline
        .handler = (void (*)(const void *))handler  // clang newline
    };
  };

  void OnStopByteReceived(std::function<void()> onPacketReceivedHandler) {
    m_OnPacketReceivedHandler = onPacketReceivedHandler;
  };

private:
  struct RequestTypeInfo {
    uint8_t id;
    uint8_t dataSize;
    std::function<void(const void *)> handler;
  };

  std::map<uint8_t, RequestTypeInfo> m_Handlers;
  std::function<void()> m_OnPacketReceivedHandler;
  std::function<void(const uint8_t)> m_StreamingHandler;

  bool m_WaitingStartCondition;
  bool m_WaitingFinishCondition;
  bool m_Streaming;
  uint8_t m_Data[BUFFER_SIZE];
  uint8_t m_DataCount;
  uint8_t m_DeserializedData[BUFFER_SIZE / 2];
  uint8_t m_DeserializedDataCount;

  void resetToWaitingStartCondition();

  void processData();
};

struct CpuStatusRequest {
  static constexpr uint8_t type_id = 1;

  uint8_t dummy;
} __attribute__((packed));

struct CpuStatusResponse {
  static constexpr uint8_t type_id = 2;

  uint8_t CpuVer;
  uint8_t FwVer;
  uint8_t BoardRev;
  uint32_t Uptime;
} __attribute__((packed));

struct GenericStatusResponse {
  static constexpr uint8_t type_id = 3;

  uint32_t BoilerStatus;
  float ExtTemp;
  uint16_t LightValue;
} __attribute__((packed));

struct OtCommandRequest {
  static constexpr uint8_t type_id = 4;

  uint32_t Payload;
} __attribute__((packed));

struct OtCommandResponse {
  static constexpr uint8_t type_id = 5;

  uint32_t Payload;
  uint8_t ResponseStatus;
} __attribute__((packed));

struct LogRequest {
  static constexpr uint8_t type_id = 6;

  uint8_t Payload[25];
  uint8_t Length;
} __attribute__((packed));

}  // namespace opentherm
}  // namespace esphome