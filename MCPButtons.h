#ifndef MCPBUTTONS_H
#define MCPBUTTONS_H

#include <Wire.h>
#include <Adafruit_MCP23X17.h>

#define NUM_BUTTONS 16

class MCPButtons {
  public:
    enum ButtonName {
      BTN_UP,
      BTN_DOWN,
      BTN_LEFT,
      BTN_RIGHT,
      BTN_SELECT,
      BTN_5,
      BTN_6,
      BTN_7
    };

    void begin(uint8_t addr = 0x20) {
      mcp.begin_I2C(addr);

      for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        mcp.pinMode(i, INPUT_PULLUP);
        currentState[i] = mcp.digitalRead(i);
        lastState[i] = currentState[i];
        lastDebounceTime[i] = millis();
        pressStartTime[i] = 0;
        newPressed[i] = false;
        released[i] = false;
        longPressed[i] = false;
        pressedFlag[i] = false;
      }
    }

    void update() {
      uint32_t now_ = millis();

      for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        bool reading = mcp.digitalRead(i);

        if (reading != lastState[i]) {
          lastDebounceTime[i] = now_;
        }

        if ((now_ - lastDebounceTime[i]) > debounceDelay) {
          if (reading != currentState[i]) {
            currentState[i] = reading;

            if (reading == LOW) {
              // Tuş basıldı
              if (!pressedFlag[i]) {
                newPressed[i] = true;
                pressedFlag[i] = true;
              }
              pressStartTime[i] = now_;
              longPressed[i] = false;
              released[i] = false;

            } else {
              // Tuş bırakıldı
              pressedFlag[i] = false;

              // Uzun basmadan sonra kısa basmayı ve bırakmayı bastır
              if (longPressed[i]) {
                newPressed[i] = false;
                released[i] = false;
              } else {
                released[i] = true;
              }

              longPressed[i] = false;
              pressStartTime[i] = 0;
            }
          }
        }

        // Uzun basma kontrolü
        if (currentState[i] == LOW && !longPressed[i] &&
            (now_ - pressStartTime[i]) > longPressTime) {
          longPressed[i] = true;
        }

        lastState[i] = reading;
      }
    }

    // Anlık basılı mı
    bool isPressed(uint8_t b) {
      return currentState[b] == LOW;
    }

    // Yeni basma (bir kez döner)
    bool isNewPressed(uint8_t b) {
      if (newPressed[b]) {
        newPressed[b] = false;
        return true;
      }
      return false;
    }

    // Bırakıldı mı (bir kez döner)
    bool isReleased(uint8_t b) {
      if (released[b]) {
        released[b] = false;
        return true;
      }
      return false;
    }

    // Uzun basma (tek seferlik değil, basılı kaldıkça true döner)
    bool isLongPressed(uint8_t b) {
      return longPressed[b];
    }

    // Uzun basma sadece bir kez döner
    bool wasLongPressed(uint8_t b) {
      if (longPressed[b]) {
        longPressed[b] = false;
        return true;
      }
      return false;
    }

  private:
    Adafruit_MCP23X17 mcp;

    bool currentState[NUM_BUTTONS];
    bool lastState[NUM_BUTTONS];
    bool newPressed[NUM_BUTTONS];
    bool released[NUM_BUTTONS];
    bool longPressed[NUM_BUTTONS];
    bool pressedFlag[NUM_BUTTONS];

    uint32_t lastDebounceTime[NUM_BUTTONS];
    uint32_t pressStartTime[NUM_BUTTONS];

    static constexpr uint16_t debounceDelay = 30;     // ms
    static constexpr uint16_t longPressTime = 1000;   // ms
};

#endif
