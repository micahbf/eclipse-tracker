#include <FastLED.h>
#include <EasyButton.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 8
#define NP_MAIN_BUTTON 0
#define NP_PLAYER_START 2

// brightness levels are 0-255
#define BUTTON_BRIGHTNESS 64
#define PLAYER_BRIGHTNESS 32

#define MAIN_BUTTON_PIN 2
#define DBL_PRESS_DEBOUNCE 250

CRGB leds[NEOPIXEL_COUNT];

EasyButton mainButton(MAIN_BUTTON_PIN);

CRGB playerColors[] = {CRGB::Blue, CRGB::Red, CRGB::Green, CRGB::White, CRGB::Yellow, CRGB::Purple};
bool playerPassed[] = {false, false, false, false, false, false};
int prevActive = 0;
int activePlayer = 0;

unsigned long lastButtonPress = 0;

void setup() {
  FastLED.addLeds<NEOPIXEL, NEOPIXEL_PIN>(leds, NEOPIXEL_COUNT);
  mainButton.begin();
  mainButton.onPressed(nextPlayer);
  mainButton.onSequence(2, DBL_PRESS_DEBOUNCE, passPlayer);
  setPlayer();
}

void setPlayer() {
  leds[NP_MAIN_BUTTON] = playerColors[activePlayer];
  leds[NP_MAIN_BUTTON].nscale8(BUTTON_BRIGHTNESS);

  for(int player = 0; player < 6; player++) {
    int led = player + NP_PLAYER_START;
    leds[led] = playerColors[player];

    if (playerPassed[player]) {
      leds[led].nscale8(PLAYER_BRIGHTNESS / 8);
    } else {
      leds[led].nscale8(PLAYER_BRIGHTNESS);
    }
  }

  FastLED.show();
}

void nextPlayer() {
  unsigned long now = millis();
  if (now - lastButtonPress < 250) {
    lastButtonPress = now;
    return;
  }

  lastButtonPress = now;

  prevActive = activePlayer;
  do {
    activePlayer = (activePlayer + 1) % 6;
  } while(playerPassed[activePlayer]);

  setPlayer();
}

void passPlayer() {
  playerPassed[prevActive] = true;

  bool allPassed = true;
  for (int player = 0; player < 6; player++) {
    allPassed = allPassed && playerPassed[player];
  }
  if (allPassed) {
    for (int player = 0; player < 6; player++) {
      playerPassed[player] = false;
      activePlayer = 0;
      prevActive = 0;
    }
  }

  setPlayer();
}

void loop() {
  mainButton.read();
}
