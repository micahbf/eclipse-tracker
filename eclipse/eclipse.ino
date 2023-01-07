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

enum TrackerMode { numPlayerSelect, colorAssign, turnTracker };

struct TurnState {
  byte prevPlayer;
  byte activePlayer;
  int playIndex;
  byte playOrder[6];
  int numPassed;
  byte passOrder[6];

  TurnState() {
    prevPlayer = 0;
    activePlayer = 0;
    playIndex = 0;
    numPassed = 0;
  }
};

#define TURN_HISTORY_LENGTH 6
struct {
  TurnState history[TURN_HISTORY_LENGTH];
  int currentState;
  int historyLength = 0;
} turnHistory;

CRGB leds[NEOPIXEL_COUNT];

EasyButton mainButton(MAIN_BUTTON_PIN);

const CRGB playerColors[] = {CRGB::Blue, CRGB::Red, CRGB::Green, CRGB::White, CRGB::Yellow, CRGB::Purple};

int numPlayers = 6;
TrackerMode currTrackerMode = turnTracker;

unsigned long lastButtonPress = 0;

void setup() {
  // Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, NEOPIXEL_PIN>(leds, NEOPIXEL_COUNT);
  showBlank();
  mainButton.begin();
  mainButton.onPressed(handleMainSinglePress);
  mainButton.onSequence(2, DBL_PRESS_DEBOUNCE, handleMainDoublePress);

  initTurnHistory();
  showTurnTracker();
}

// void debugCurrState() {
//   TurnState &currState = getCurrentState();

//   Serial.print("\n\n");
//   Serial.print(int(&currState), HEX);
//   Serial.print("\nprevPlayer ");
//   Serial.print(currState.prevPlayer, DEC);
//   Serial.print("\nactivePlayer ");
//   Serial.print(currState.activePlayer, DEC);
//   Serial.print("\nplayIndex ");
//   Serial.print(currState.playIndex, DEC);
//   Serial.print("\nnumPassed ");
//   Serial.print(currState.numPassed, DEC);
//   Serial.print("\npassOrder ");
//   for(int i = 0; i < 6; i++) {
//     Serial.print(currState.passOrder[i], DEC);
//     Serial.print(" ");
//   }
// }

void handleMainSinglePress() {
  unsigned long now = millis();
  if (now - lastButtonPress < 250) {
    lastButtonPress = now;
    return;
  }

  lastButtonPress = now;

  switch(currTrackerMode) {
    case turnTracker:
      nextPlayer();
      break;
  }
}

void handleMainDoublePress() {
  switch(currTrackerMode) {
    case turnTracker:
      passPrevPlayer();
      break;
  }
}

void showBlank() {
  for(int led = 0; led < NEOPIXEL_COUNT; led++) {
    leds[led] = CRGB::Black;
  }
  FastLED.show();
}

void showTurnTracker() {
  TurnState &turnState = getCurrentState();

  leds[NP_MAIN_BUTTON] = playerColors[turnState.activePlayer];
  leds[NP_MAIN_BUTTON].nscale8(BUTTON_BRIGHTNESS);

  for(int i = 0; i < numPlayers; i++) {
    int led = i + NP_PLAYER_START;
    byte player = turnState.playOrder[i];
    leds[led] = playerColors[player];

    if (isPlayerPassed(turnState, player)) {
      leds[led].nscale8(2);
    } else {
      leds[led].nscale8(PLAYER_BRIGHTNESS);
    }
  }

  FastLED.show();
}

void nextPlayer() {
  TurnState &newState = advanceTurnState();
  newState.prevPlayer = newState.activePlayer;

  do {
    newState.playIndex = (newState.playIndex + 1) % numPlayers;
    newState.activePlayer = newState.playOrder[newState.playIndex];
  } while(isPlayerPassed(newState, newState.activePlayer));

  // debugCurrState();
  showTurnTracker();
}

void passPrevPlayer() {
  TurnState &newState = advanceTurnState();
  newState.passOrder[newState.numPassed] = newState.prevPlayer;
  newState.numPassed++;
  

  if (newState.numPassed == numPlayers) {
    for (int i = 0; i < numPlayers; i++) {
      newState.playOrder[i] = newState.passOrder[i];
    }
    newState.playIndex = 0;
    newState.numPassed = 0;
  }

  // debugCurrState();
  showTurnTracker();
}

bool isPlayerPassed(TurnState &turnState, byte player) {
  if (turnState.numPassed == 0) {
    return false;
  }

  for (int i = 0; i < turnState.numPassed; i++) {
    if (turnState.passOrder[i] == player) {
      return true;
    }
  }

  return false;
}

TurnState& getCurrentState() {
  return turnHistory.history[turnHistory.currentState];
}

TurnState& advanceTurnState() {
  int nextStateIdx = (turnHistory.currentState + 1) % TURN_HISTORY_LENGTH;
  turnHistory.history[nextStateIdx] = turnHistory.history[turnHistory.currentState];
  turnHistory.currentState = nextStateIdx;
  if (turnHistory.historyLength < TURN_HISTORY_LENGTH) {
    turnHistory.historyLength++;
  }

  TurnState &nextState = turnHistory.history[nextStateIdx];
  return nextState;
}

void initTurnHistory() {
  for (int i; i < TURN_HISTORY_LENGTH; i++) {
    turnHistory.history[i] = TurnState();
  }
  turnHistory.currentState = 0;

  for (int i; i < numPlayers; i++) {
    turnHistory.history[0].playOrder[i] = byte(i);
  }
}

void loop() {
  mainButton.read();
}
