#include <FastLED.h>
#include <EasyButton.h>

#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 8
#define NP_MAIN_BUTTON 0
#define NP_PLAYER_START 2

// brightness levels are 0-255
#define BUTTON_BRIGHTNESS 64
#define PLAYER_BRIGHTNESS 32
#define PASSED_BRIGHTNESS 4

#define MAIN_BUTTON_PIN 2
#define UNDO_BUTTON_PIN 3
#define DBL_PRESS_DEBOUNCE 250

enum TrackerMode { numPlayerSelect,
                   randomColorAssign,
                   turnTracker };

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
EasyButton undoButton(UNDO_BUTTON_PIN);

const CRGB playerColors[] = { CRGB::Blue, CRGB::Red, CRGB::Green, CRGB::White, CRGB::Yellow, CRGB::Purple };

int numPlayers = 4;
TrackerMode currTrackerMode = numPlayerSelect;

unsigned long lastButtonPress = 0;
unsigned long lastTick = 0;

void setup() {
  // Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, NEOPIXEL_PIN>(leds, NEOPIXEL_COUNT).setCorrection(TypicalLEDStrip);
  showBlank();
  mainButton.begin();
  mainButton.onPressed(handleMainSinglePress);
  mainButton.onSequence(2, DBL_PRESS_DEBOUNCE, handleMainDoublePress);
  undoButton.begin();
  undoButton.onPressed(handleUndoSinglePress);

  initTurnHistory();
  showNumPlayerSelect();
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

  switch (currTrackerMode) {
    case turnTracker:
      nextPlayer();
      break;
    case numPlayerSelect:
      incNumPlayers();
      break;
    case randomColorAssign:
      assignColor();
      break;
  }
}

void handleMainDoublePress() {
  switch (currTrackerMode) {
    case turnTracker:
      passPrevPlayer();
      break;
    case numPlayerSelect:
      numPlayersConfirm();
      break;
  }
}

void handleUndoSinglePress() {
  switch (currTrackerMode) {
    case turnTracker:
      trackerUndo();
      break;
  }
}

void showBlank() {
  for (int led = 0; led < NEOPIXEL_COUNT; led++) {
    leds[led] = CRGB::Black;
  }
  FastLED.show();
}

void showNumPlayerSelect() {
  leds[NP_MAIN_BUTTON] = CRGB::Plum;
  leds[NP_MAIN_BUTTON].nscale8(BUTTON_BRIGHTNESS);

  for (int i = 0; i < 6; i++) {
    int led = i + NP_PLAYER_START;
    if (i < numPlayers) {
      leds[led] = CRGB::Plum;
      leds[led].nscale8(PLAYER_BRIGHTNESS);
    } else {
      leds[led] = CRGB::Black;
    }
  }
  FastLED.show();
}

void showColorAssign() {
  TurnState &turnState = getCurrentState();

  leds[NP_MAIN_BUTTON] = playerColors[turnState.activePlayer];
  leds[NP_MAIN_BUTTON].nscale8(BUTTON_BRIGHTNESS);

  for (int i = 0; i < numPlayers; i++) {
    int led = i + NP_PLAYER_START;
    if (i <= turnState.activePlayer) {
      leds[led] = playerColors[i];
      leds[led].nscale8(PLAYER_BRIGHTNESS);
    } else {
      leds[led] = CRGB::Black;
    }
  }
}

void showTurnTracker() {
  TurnState &turnState = getCurrentState();

  leds[NP_MAIN_BUTTON] = playerColors[turnState.activePlayer];
  leds[NP_MAIN_BUTTON].nscale8(BUTTON_BRIGHTNESS);

  for (int i = 0; i < numPlayers; i++) {
    int led = i + NP_PLAYER_START;
    byte player = turnState.playOrder[i];
    leds[led] = playerColors[player];

    if (isPlayerPassed(turnState, player)) {
      leds[led].nscale8(PASSED_BRIGHTNESS);
    } else {
      leds[led].nscale8(PLAYER_BRIGHTNESS);
    }
  }

  FastLED.show();
}

void incNumPlayers() {
  if (numPlayers == 6) {
    numPlayers = 2;
  } else {
    numPlayers++;
  }

  showNumPlayerSelect();
}

void numPlayersConfirm() {
  // undo the single last button press
  if (numPlayers == 2) {
    numPlayers = 6;
  } else {
    numPlayers--;
  }

  currTrackerMode = randomColorAssign;
  showBlank();
  showColorAssign();
}

void randomAssignTick() {
  TurnState &turnState = getCurrentState();
  byte nextColor = turnState.playOrder[turnState.playIndex];
  bool colorTaken;
  do {
    colorTaken = false;
    nextColor = nextColor == 5 ? 0 : nextColor + 1;

    for (int i = 0; i < turnState.playIndex; i++) {
      if (turnState.playOrder[i] == nextColor) {
        colorTaken = true;
      }
    }
  } while (colorTaken);

  turnState.playOrder[turnState.playIndex] = nextColor;
  leds[NP_MAIN_BUTTON] = playerColors[nextColor];
  leds[NP_MAIN_BUTTON].nscale8(BUTTON_BRIGHTNESS);
  leds[NP_PLAYER_START + turnState.playIndex] = playerColors[nextColor];
  leds[NP_PLAYER_START + turnState.playIndex].nscale8(PLAYER_BRIGHTNESS);
  FastLED.show();
}

void assignColor() {
  TurnState &turnState = getCurrentState();
  if (turnState.playIndex == 4 || turnState.playIndex == numPlayers - 1) {
    currTrackerMode = turnTracker;
    turnState.playIndex = 0;
    turnState.activePlayer = turnState.playOrder[0];    
    showBlank();
    showTurnTracker();
  } else {
    turnState.playIndex++;
  }
}

void nextPlayer() {
  TurnState &newState = advanceTurnState();
  newState.prevPlayer = newState.activePlayer;

  do {
    newState.playIndex = (newState.playIndex + 1) % numPlayers;
    newState.activePlayer = newState.playOrder[newState.playIndex];
  } while (isPlayerPassed(newState, newState.activePlayer));

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
    newState.prevPlayer = newState.activePlayer;
    newState.activePlayer = newState.playOrder[0];
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

void trackerUndo() {
  if (turnHistory.historyLength == 0) {
    return;
  }

  int newState;
  if (turnHistory.currentState - 1 < 0) {
    newState = TURN_HISTORY_LENGTH - 1;
  } else {
    newState = turnHistory.currentState - 1;
  }

  turnHistory.currentState = newState;
  turnHistory.historyLength--;
  showTurnTracker();
}

TurnState &getCurrentState() {
  return turnHistory.history[turnHistory.currentState];
}

TurnState &advanceTurnState() {
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
  for (int i = 0; i < TURN_HISTORY_LENGTH; i++) {
    turnHistory.history[i] = TurnState();
  }
  turnHistory.currentState = 0;

  for (int i = 0; i < numPlayers; i++) {
    turnHistory.history[0].playOrder[i] = byte(i);
  }
}

void loop() {
  mainButton.read();
  undoButton.read();

  unsigned long now = millis();
  if (currTrackerMode == randomColorAssign && now != lastTick && now % 50 == 0) {
    randomAssignTick();
  }
  lastTick = now;
}
