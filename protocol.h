#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

enum MessageType {
  FCM = 1, // primer mensaje de comunicacion
  RM  = 2, // mensaje regular
  KUM = 3, // mensaje de actualizacion de la clave o llave
  LCM = 4  // ultimo mensaje de comunicacion
};

struct MessagePacket {
  uint16_t nodeID;
  uint8_t  type;
  uint32_t psn;
  uint8_t  payloadLen;
  uint8_t  payload[32];
};

inline uint64_t generateKey64() {
  uint64_t k1 = esp_random();
  uint64_t k2 = esp_random();
  return (k1 << 32) | k2;
}

inline void applyPolymorphicCipher(uint8_t* data, uint8_t len, uint64_t key, uint32_t psn) {
  uint8_t* keyBytes = (uint8_t*)&key;
  for (uint8_t i = 0; i < len; i++) {
    uint8_t k = keyBytes[i % 8];
    uint8_t val = data[i] ^ k;
    uint8_t shift = (psn + i) % 8;
    data[i] = (val << shift) | (val >> (8 - shift));
  }
}

inline void applyPolymorphicDecipher(uint8_t* data, uint8_t len, uint64_t key, uint32_t psn) {
  uint8_t* keyBytes = (uint8_t*)&key;
  for (uint8_t i = 0; i < len; i++) {
    uint8_t k = keyBytes[i % 8];
    uint8_t shift = (psn + i) % 8;
    uint8_t val = (data[i] >> shift) | (data[i] << (8 - shift));
    data[i] = val ^ k;
  }
}

#endif