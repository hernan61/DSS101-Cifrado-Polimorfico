# Caso de Estudio 2 - DSS101 (UDB)

**Materia:** Diseño de Sistemas de Seguridad en Redes de Datos  
**Universidad Don Bosco**  

### Integrantes del Grupo:
* **Hernán Alberto Palacios Díaz** - PD231857
* **Idalia Gabriela Escobar Alvarado** - EA240718
* **Dominick Jerome Cordova Hernandez** - CH252698
* **Fernando Alberto Barahona Castro** – BC221567

---

## 1. Enlaces del Proyecto
* **Simulación en Vivo (Wokwi):** https://wokwi.com/projects/474943143167192065
---

## 2. Escenario Utilizado
El escenario de simulación fue construido en Wokwi (`diagram.json`) y consta de:
* 2 Microcontroladores **ESP32 DevKit v4** (`esp_tx` y `esp_rx`) simulados en arquitectura dual.
* Conexión inalámbrica a la red virtual **Wokwi-GUEST** mediante sockets UDP (puerto 4210).
* Mapeo del Monitor Serial a 115200 baudios para la visualización de tramas cifradas.

---

## 3. Estructura del Código
* `sketch.ino`: Programa principal en C++ que contiene la lógica integrada de **ambos nodos (Transmisor y Receptor)** mediante funciones `transmitirNodoTX()` y `recibirNodoRX()`.
* `protocol.h`: Definición de la estructura de la trama (`MessagePacket`), enum de mensajes (`FCM`, `RM`, `KUM`, `LCM`) y funciones de cifrado/descifrado polimórfico (XOR + Shift).
* `diagram.json`: Definición gráfica y técnica del escenario en Wokwi.

---

## 4. Instrucciones de Ejecución
1. Abrir el enlace de la simulación en Wokwi indicado arriba.
2. Asegurarse de que los archivos `sketch.ino`, `protocol.h` y `diagram.json` estén cargados.
3. Hacer clic en el botón **Play** (triángulo verde) en la parte superior del simulador.
4. Desplegar el **Serial Monitor** para observar el envío y recepción de datos cifrados en tiempo real.
