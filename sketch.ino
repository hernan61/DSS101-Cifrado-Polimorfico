#include <WiFi.h>
#include <WiFiUdp.h>
#include "protocol.h" // Cabecera con la estructura MessagePacket y funciones XOR+Shift

// Parametros de red para el simulador Wokwi
const char* ssid = "Wokwi-GUEST"; // AP abierto por defecto en Wokwi
const char* password = "";

// Instancias UDP para simular la comunicacion bidireccional en la misma interfaz
WiFiUDP udpTX;
WiFiUDP udpRX;

const unsigned int portRX = 4210; // Puerto socket UDP de escucha
IPAddress localIP;

// Llave inicial de 64 bits compartida para la sesion
uint64_t llaveActual64 = 0xA1B2C3D4E5F60718ULL;
uint32_t numeroSecuenciaPSN = 1000; // Contador de secuencia para evitar ataques Replay

void setup() {
  Serial.begin(115200);
  delay(1000); // Espera para estabilizar el monitor serial

  Serial.println("\n==================================================");
  Serial.println("  SISTEMA IoT DUAL ESP32 - RED WIFI (Wokwi-GUEST) ");
  Serial.println("==================================================");

  // Conexión Wi-Fi al punto de acceso virtual
  WiFi.begin(ssid, password);
  Serial.print("[ESP32_TX & RX] Conectando a la red Wi-Fi Wokwi-GUEST...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  localIP = WiFi.localIP();
  Serial.println("\n¡Ambos Nodos ESP32 conectados exitosamente a la Red!");
  Serial.print("Direccion IP del Enlace Wi-Fi: ");
  Serial.println(localIP);

  // Abrir puerto UDP en el nodo Receptor (RX)
  udpRX.begin(portRX);
  Serial.printf("[ESP32_RX] Escuchando paquetes UDP en el puerto %d...\n\n", portRX);
}

// Funcion del Transmisor: Empaqueta, cifra con algoritmo polimorfico y envia por UDP
void transmitirNodoTX(MessageType tipoMensaje, const char* textoPlano) {
  MessagePacket paquete;
  paquete.nodeID = 101; // ID identificador de este ESP32
  paquete.type = (uint8_t)tipoMensaje;
  paquete.psn = numeroSecuenciaPSN++; // Auto-incremento del PSN por paquete
  paquete.payloadLen = strlen(textoPlano);
  
  // Limpiar el buffer del payload antes de copiar los datos
  memset(paquete.payload, 0, sizeof(paquete.payload));
  memcpy(paquete.payload, textoPlano, paquete.payloadLen);

  // Si el mensaje es KUM, regeneramos la clave de 64 bits sobre la marcha
  if (tipoMensaje == KUM) {
    llaveActual64 = generateKey64(); // Genera nuevo valor aleatorio usando esp_random()
    Serial.print("\n>>> [ESP32_TX - Wi-Fi] ACTUALIZACION DE LLAVE: 0x");
    Serial.print((uint32_t)(llaveActual64 >> 32), HEX);
    Serial.print((uint32_t)llaveActual64, HEX);
    Serial.println(" <<<");
  }

  // Aplicar Cifrado Polimorfico (Mascara XOR + Rotacion dinamica dependiente de PSN)
  applyPolymorphicCipher(paquete.payload, paquete.payloadLen, llaveActual64, paquete.psn);

  Serial.printf("[ESP32_TX -> Wi-Fi] Enviando Tipo: %d | PSN: %u | Texto Plano: '%s'\n", tipoMensaje, paquete.psn, textoPlano);

  // Transmision de la estructura binaria por el socket UDP (loopback local en Wokwi)
  udpTX.beginPacket(IPAddress(127, 0, 0, 1), portRX);
  udpTX.write((uint8_t*)&paquete, sizeof(MessagePacket));
  udpTX.endPacket();
}

// Funcion del Receptor: Lee el socket UDP, valida el paquete y descifra la carga util
void recibirNodoRX() {
  int packetSize = udpRX.parsePacket();
  
  // Validar que el tamano del paquete recibido corresponda a la estructura MessagePacket
  if (packetSize >= sizeof(MessagePacket)) {
    MessagePacket paqueteRecibido;
    udpRX.read((char*)&paqueteRecibido, sizeof(MessagePacket));

    char textoDescifrado[33] = {0}; // Buffer temporal terminado en NULL
    memcpy(textoDescifrado, paqueteRecibido.payload, paqueteRecibido.payloadLen);

    // Aplicar Descifrado Polimorfico Inverso (Shift a la derecha + XOR)
    applyPolymorphicDecipher((uint8_t*)textoDescifrado, paqueteRecibido.payloadLen, llaveActual64, paqueteRecibido.psn);

    // Impresion de resultados en consola
    Serial.println("  --------------------------------------------------");
    Serial.printf("  [ESP32_RX <- Wi-Fi] Paquete UDP Recibido de NodeID: %d\n", paqueteRecibido.nodeID);
    Serial.printf("  [ESP32_RX] Tipo: %d | PSN: %u | Cifrado OK\n", paqueteRecibido.type, paqueteRecibido.psn);
    Serial.printf("  [ESP32_RX] Mensaje Descifrado: \"%s\"\n", textoDescifrado);
    Serial.println("  --------------------------------------------------\n");
  }
}

void loop() {
  // --- RAFAGA DE MENSAJES M2M ---

  // 1. Mensaje de Apertura de Sesion (FCM)
  transmitirNodoTX(FCM, "INIT_SESSION");
  delay(100); // Pauta breve para permitir procesamiento del socket RX
  recibirNodoRX();
  delay(3000);

  // 2. Transmision de Datos Regulares de Sensores (RM)
  transmitirNodoTX(RM, "DATA_SENSOR_24C");
  delay(100);
  recibirNodoRX();
  delay(3000);

  // 3. Rotacion y Actualizacion de Llave Criptografica (KUM)
  transmitirNodoTX(KUM, "KEY_UPDATE_64");
  delay(100);
  recibirNodoRX();
  delay(3000);

  // 4. Mensaje de Cierre de Sesion (LCM)
  transmitirNodoTX(LCM, "END_SESSION");
  delay(100);
  recibirNodoRX();
  delay(5000); // Espera extendida antes de reiniciar la secuencia
}