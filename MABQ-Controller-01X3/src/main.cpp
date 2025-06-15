#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include <string>
#include <cstdint>
#include <functional>
#include <set>

#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets
#include <Bounce2.h>          // https://github.com/thomasfredericks/Bounce2

//
#define SERIAL_BAUD 115200

#define BTNS_LONG_PRESS_TIME 750 // ms
#define BTNS_DEBOUNCE_TIME 5     // ms
#define BTN1_PIN 5
#define BTN2_PIN 17
#define BTN3_PIN 0
#define BTN4_PIN 2

#define POT1_PIN 34
#define POT2_PIN 35

#define JOY1X_PIN 36
#define JOY1Y_PIN 39
#define JOY1BTN_PIN 18

#define JOY2X_PIN 32
#define JOY2Y_PIN 33
#define JOY2BTN_PIN 13

#define STATUS_LED_PIN 16

#define WIFI_SSID "MABQ-25X-Controller"
#define WIFI_PASSWORD "12345678"

// WS IP: ws://MDNS_NAME.local:WS_PORT/
#define MDNS_NAME "MABQ-25X-Controller"
#define WS_MAX_CLIENTS 2
#define WS_PORT 81

// Using NRF24L01+
#define NRF24_CE_PIN 5 // Chip Enable
#define NRF24_CSN_PIN 4 // Chip Select Not
#define NRF24_SCK_PIN 18 // Serial Clock
#define NRF24_MISO_PIN 19 // Master In Slave Out
#define NRF24_MOSI_PIN 23 // Master Out Slave In
#define NRF24_IRQ_PIN 99  // Interrupt (Not used)

//
class Button
{
protected:
  short id;
  uint8_t pin;
  Bounce2::Button b2Obj;
  std::function<void()> onClick;
  std::function<void()> onPress;
  std::function<void()> onRelease;
  std::function<void()> onLongPress;

public:
  Button(short id, uint8_t pin) : id(id), pin(pin) {}

  void setup()
  {
    b2Obj.attach(pin, INPUT_PULLUP);
    b2Obj.interval(BTNS_DEBOUNCE_TIME);
    b2Obj.setPressedState(LOW);
  }

  void update()
  {
    b2Obj.update();

    if (b2Obj.pressed() && onPress)
      onPress();

    if (b2Obj.released())
    {
      if (b2Obj.previousDuration() < BTNS_LONG_PRESS_TIME)
      {
        if (onClick)
          onClick();
      }
      else
      {
        if (onLongPress)
          onLongPress();
      }

      if (onRelease)
        onRelease();
    }
  }

  bool isPressed() { return b2Obj.isPressed(); }

  short getId() { return id; }

  void setOnClick(std::function<void()> callback) { onClick = callback; }
  void setOnPress(std::function<void()> callback) { onPress = callback; }
  void setOnRelease(std::function<void()> callback) { onRelease = callback; }
  void setOnLongPress(std::function<void()> callback) { onLongPress = callback; }

  void resetEvents()
  {
    onClick = nullptr;
    onPress = nullptr;
    onRelease = nullptr;
    onLongPress = nullptr;
  }
};

class Potentiom
{
private:
  short id;
  uint8_t pin;
  uint8_t value = 0;

public:
  Potentiom(short id, uint8_t pin) : id(id), pin(pin) {}

  void update() { value = map(analogRead(pin), 0, 4095, 0, 254); }

  short getId() { return id; }

  uint8_t getValue() { return value; }
};

class Led
{
private:
  short id;
  uint8_t pin;

public:
  Led(short id, uint8_t pin) : id(id), pin(pin) {}

  void setup()
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  void update() {}

  void turnOn() { digitalWrite(pin, HIGH); }

  void turnOff() { digitalWrite(pin, LOW); }

  short getId() { return id; }
};

class Joystick
{
private:
  short id;
  Potentiom axisX;
  Potentiom axisY;
  Button btn;

public:
  Joystick(short id, uint8_t pinX, uint8_t pinY, uint8_t pinBtn)
      : id(id), axisX(id * 100 + 1, pinX), axisY(id * 100 + 2, pinY), btn(id * 100 + 1, pinBtn) {}

  void setup() { btn.setup(); }
  void update()
  {
    axisX.update();
    axisY.update();
    btn.update();
  }

  short getId() { return id; }

  short getXValue() { return map(axisX.getValue(), 0, 254, 254, 0); }
  short getYValue() { return axisY.getValue(); }
  bool isBtnPressed() { return btn.isPressed(); }
};

class Status
{
private:
  short id;
  std::string name;
  std::function<void()> onBegin;
  std::function<void()> onLoop;
  std::function<void(std::string)> onError;
  std::string errorName;
  std::function<void(short)> onExit;

public:
  Status(short id, std::string name) : id(id), name(name) {}

  short getId() { return id; }
  std::string getName() { return name; }
  std::string getErrorName() { return errorName; }

  void setOnBegin(std::function<void()> callback) { onBegin = callback; }
  void executeOnBegin() { if (onBegin) onBegin(); }

  void setOnLoop(std::function<void()> callback) { onLoop = callback; }
  void executeOnLoop() { if (onLoop) onLoop(); }

  void setOnError(std::function<void(std::string)> callback) { onError = callback; }
  void executeOnError(std::string error)
  {
    if (onError)
    {
      errorName = error;
      onError(error);
      errorName = "";
    }
  }

  void newError(std::string error)
  {
    errorName = error;
    if (onError) onError(error);
  }

  void setOnExit(std::function<void(short)> callback) { onExit = callback; }
  void executeOnExit(short statusId)
  {
    if (onExit)
    {
      onExit(statusId);
      errorName = "";
    }
  }
};

class StatusManager
{
private:
  short id;

public:
  Status activeStatus;
  Status status1;
  Status status2;
  Status status3;
  Status status4;

  StatusManager(short id):
    id(id), activeStatus(0, "None"),
    status1(1, "Start"), status2(2, "Wifi setup"), 
    status3(3, "WS setup"), status4(4, "Data sending to WS") {}

  short getId() { return id; }

  void set(short statusId)
  {
    if (activeStatus.getId() != 0)
    {
      activeStatus.executeOnExit(statusId);
    }

    switch (statusId)
    {
    case 1:
      activeStatus = status1;
      break;
    case 2:
      activeStatus = status2;
      break;
    case 3:
      activeStatus = status3;
      break;
    case 4:
      activeStatus = status4;
      break;
    default:
      break;
    }
    activeStatus.executeOnBegin();
  }

  void loop()
  {
    if (activeStatus.getId() != 0)
    {
      activeStatus.executeOnLoop();
      if (activeStatus.getErrorName() != "")
      {
        activeStatus.executeOnError(activeStatus.getErrorName());
      }
    }
    else
    {
      Serial.println("[StatusManager] No active status");
    }
  }
};

class WebSocketServer
{
private:
  WebSocketsServer webSocket;
  short port;
  short connectedClients = 0;
  const short maxClients = WS_MAX_CLIENTS;
  std::set<uint8_t> ignoredClients;
  std::function<void(uint8_t, std::string)> onMessage;
  std::function<void(uint8_t)> onConnected;
  std::function<void(uint8_t)> onDisconnected;
  std::function<void(uint8_t, std::string)> onError;
  std::function<void(uint8_t)> onClientMaxReached;
  std::function<void()> onLoop;

public:
  WebSocketServer(short port) : port(port), webSocket(port) {}

  void begin()
  {
    this->webSocket.begin();
    this->webSocket.onEvent(
      [this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
      {
        switch (type) 
        {
          case WStype_TEXT:
            if (onMessage) onMessage(num, std::string((char*)payload, length));
            break;

          case WStype_CONNECTED:
            if (connectedClients >= maxClients) {
              if (onClientMaxReached) onClientMaxReached(num);

              ignoredClients.insert(num);
              webSocket.disconnect(num);
              break;
            }

            connectedClients++;
            if (onConnected) onConnected(num);
            break;

          case WStype_DISCONNECTED:
            if (ignoredClients.count(num)) {
              ignoredClients.erase(num);
              break;
            }

            connectedClients--;
            if (onDisconnected) onDisconnected(num);
            break;

          case WStype_ERROR:
            if (onError) {
              bool printable = true;
              for (size_t i = 0; i < length; ++i) {
                if (!isprint(payload[i]) && payload[i] != '\n' && payload[i] != '\r' && payload[i] != '\t') {
                  printable = false;
                  break; // Exits the loop
                }
              }
              std::string error = (printable && length > 0) ? std::string((char*)payload, length) : "Unknown error";
              onError(num, error);
            }
            break;

          default:
            break;
        } 
      }
    );
  }

  void loop()
  {
    this->webSocket.loop();
    if (onLoop) onLoop();
  }
  void send(uint8_t num, std::string message) { webSocket.sendTXT(num, message.c_str()); }
  void sendAll(std::string message) { webSocket.broadcastTXT(message.c_str()); }

  short getConnectedClients() { return connectedClients; }
  short getMaxClients() { return maxClients; }

  void setOnMessage(std::function<void(uint8_t, std::string)> callback) { onMessage = callback; }
  void setOnConnected(std::function<void(uint8_t)> callback) { onConnected = callback; }
  void setOnDisconnected(std::function<void(uint8_t)> callback) { onDisconnected = callback; }
  void setOnError(std::function<void(uint8_t, std::string)> callback) { onError = callback; }
  void setOnClientMaxReached(std::function<void(uint8_t)> callback) { onClientMaxReached = callback; }
  void setOnLoop(std::function<void()> callback) { onLoop = callback; }
};

//
Button btn1(1, BTN1_PIN);
Button btn2(2, BTN2_PIN);
Button btn3(3, BTN3_PIN);
Button btn4(4, BTN4_PIN);

Potentiom pot1(1, POT1_PIN);
Potentiom pot2(2, POT2_PIN);

Led statusLed(1, STATUS_LED_PIN);

Joystick joy1(1, JOY1X_PIN, JOY1Y_PIN, JOY1BTN_PIN);
Joystick joy2(2, JOY2X_PIN, JOY2Y_PIN, JOY2BTN_PIN);

StatusManager status(1);

WiFiManager wifiManager;

WebSocketServer webSocket(WS_PORT);

//
void setup()
{
  Serial.begin(SERIAL_BAUD);

  btn1.setup();
  btn2.setup();
  btn3.setup();
  btn4.setup();

  statusLed.setup();

  joy1.setup();
  joy2.setup();

  status.status1.setOnBegin(
    [&]()
    {
        Serial.println("[S1] \"Start\"'s status started");
        Status& thisStatus = status.activeStatus;

        // First time buttons cleanup
        btn1.resetEvents();
        btn2.resetEvents();
        btn3.resetEvents();
        btn4.resetEvents();

        // Setups
        btn1.setOnClick([&]() { Serial.println("[S1] Button 1 pressed"); status.set(2); });
        btn2.setOnClick([&]() { Serial.println("[S1] Button 2 pressed, screen will not be setup."); });

        thisStatus.setOnExit(
          [&](short nextStatusId) 
          {
            btn1.resetEvents();
            btn2.resetEvents();
            btn3.resetEvents();
            btn4.resetEvents();

            if (nextStatusId == 2) {
              Serial.println("[S1] Switching to S2: \"Wifi setup\"");
            }

            Serial.println("[S1] \"Start\"'s status exited");
            return;
          }
        );

      Serial.println("[S1] \"Start\"'s status setup ended");
      return; 
    }
  );

  status.status2.setOnBegin(
    [&]()
    {
      Serial.println("[S2] \"Wifi setup\"'s status started");
      Status& thisStatus = status.activeStatus;
      bool isWiFiSetup = false;

      // Events setups
      wifiManager.setAPCallback(
        [&](WiFiManager *myWiFiManager) 
        {
          // Executes when the default WiFi connection has failed 
          Serial.println("[S2] Failed to connect to WiFi, entering config mode");
        }
      );

      btn1.setOnClick(
        [&]() 
        {
          if (isWiFiSetup) 
          {
            Serial.println("[S2] Button 1 pressed");
            status.set(3);
          } 
          else 
          {
            Serial.println("[S2] Button 1 pressed, but WiFi setup is not done yet");
          }
        }
      );
      btn2.setOnClick(
        [&]() 
        {
          if (isWiFiSetup) {
            Serial.println("[S2] Button 2 pressed");
            status.set(2);
          } else {
            Serial.println("[S2] Button 2 pressed, but WiFi setup is not done yet");
          }
        }
      );

      // Status setups
      thisStatus.setOnError(
        [&](std::string error) 
        {
          if (error == "WiFi_connection_failed") 
          {
            Serial.println("[S2] WiFi connection failed, restarting...");
            ESP.restart();
          } 
          else if (error == "MDNS_setup_failed") 
          {
            Serial.println("[S2] Error setting up MDNS responder, restarting...");
            ESP.restart();
          }
        }
      );

      thisStatus.setOnExit(
        [&](short nextStatusId) 
        {
          btn1.resetEvents();
          btn2.resetEvents();
          btn3.resetEvents();
          btn4.resetEvents();

          if (nextStatusId == 3) 
          {
            Serial.println("[S2] Switching to S3: \"WS setup\"");
          } 
          else if (nextStatusId == 2) 
          {
            statusLed.turnOff();
            WiFi.disconnect();
            wifiManager.resetSettings();
            Serial.println("[S2] Resetting WiFi settings");
          }
          
          Serial.println("[S2] \"Wifi setup\"'s status exited");
          return;
        }
      );

      Serial.println("[S2] \"Wifi setup\"'s status setup ended");

      // Wifi setup
      if (!wifiManager.autoConnect(WIFI_SSID, WIFI_PASSWORD)) 
      {
        thisStatus.newError("WiFi_connection_failed");
        return;
      }
      Serial.println("[S2] Connected to WiFi");
      Serial.println("[S2] Starting mDNS responder");

      if (!MDNS.begin(MDNS_NAME))
      {
        thisStatus.newError("MDNS_setup_failed");
        return;
      }
      Serial.println("[S2] mDNS responder started");

      isWiFiSetup = true;
      statusLed.turnOn();

      return; 
    }
  );

  status.status3.setOnBegin(
    [&]()
    {
      Serial.println("[S3] Status 3 started");

      webSocket.begin();
      Serial.println("[S3] WebSocket server started, waiting for a client...");

      webSocket.setOnConnected(
        [](uint8_t num) 
        {
          Serial.printf("[WS] Client %u connected\n", num);
          webSocket.send(num, "[\"message\", \"Hello from ESP32!\"]");
          
          Serial.println("[S3] Client connected, switching to S4");
          status.set(4);
        }
      ); 
    }
  );

  status.status4.setOnBegin(
    [&]()
    {
      Serial.println("[S4] Status 4 started");

      // Setups
      webSocket.setOnConnected(
        [&](uint8_t num) 
        {
          Serial.printf("[WS] Client %u connected\n", num);
        }
      );
      webSocket.setOnDisconnected(
        [&](uint8_t num) 
        {
          Serial.printf("[WS] Client %u disconnected\n", num);

          if (webSocket.getConnectedClients() == 0) {
            Serial.println("[S4] No clients connected, switching to S3");
            status.set(3);
          }
        }
      );
      webSocket.setOnError(
        [&](uint8_t num, std::string error) 
        {
          Serial.printf("[WS] Error from client %u: %s\n", num, error.c_str());
        }
      );
      webSocket.setOnClientMaxReached(
        [&](uint8_t num) 
        {
          webSocket.send(num, "[\"error\", \"Maximum client limit reached\"]");
          Serial.printf(
            "[WS] Client %u disconnected because the maximum limit of %d clients was reached.\n", 
            num, webSocket.getMaxClients()
          );
        }
      );

      // Status setups
      status.activeStatus.setOnLoop(
          [&]()
          {
            std::string jsonString = "{";

            jsonString += "\"type\": \"hardwareData\",";
            jsonString += "\"data\": {";

            jsonString += "\"pots\": [";
            jsonString += std::to_string(pot1.getValue()) + ",";
            jsonString += std::to_string(pot2.getValue());
            jsonString += "],";

            jsonString += "\"btns\": [";
            jsonString += std::to_string(btn1.isPressed()) + ",";
            jsonString += std::to_string(btn2.isPressed()) + ",";
            jsonString += std::to_string(btn3.isPressed()) + ",";
            jsonString += std::to_string(btn4.isPressed());
            jsonString += "],";

            jsonString += "\"joys\": [";
            jsonString += "[";
            jsonString += std::to_string(joy1.getXValue()) + ",";
            jsonString += std::to_string(joy1.getYValue()) + ",";
            jsonString += std::to_string(joy1.isBtnPressed());
            jsonString += "],";
            jsonString += "[";
            jsonString += std::to_string(joy2.getXValue()) + ",";
            jsonString += std::to_string(joy2.getYValue()) + ",";
            jsonString += std::to_string(joy2.isBtnPressed());
            jsonString += "]";
            jsonString += "]";

            jsonString += "}"; // cierra "data"
            jsonString += "}"; // cierra todo
            webSocket.sendAll(jsonString);
          });

      status.activeStatus.setOnExit(
        [](short nextStatusId) 
        {
          btn1.resetEvents();
          btn2.resetEvents();
          btn3.resetEvents();
          btn4.resetEvents();

          Serial.println("[S4] \"Data sending to WS\"'s status exited");
          return;
        }
      );

      Serial.println("[S4] \"Data sending to WS\"'s status setup ended");
      return; 
    }
  );

  // Program start
  status.set(1);
}

void loop()
{
  status.loop();

  btn1.update();
  btn2.update();
  btn3.update();
  btn4.update();

  pot1.update();
  pot2.update();

  statusLed.update();

  joy1.update();
  joy2.update();

  webSocket.loop();

  delay(50);
}
