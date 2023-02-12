This repo contains everything needed to make an Eclipse turn tracker.

![Eclipse turn tracker](/tracker.jpeg)

## Operation

The tracker has a few modes, from startup:

1. Select number of players

- hit the arcade button to change the number of players, then double tap to confirm

2. Random draft

- Species are drafted randomly. Each player hits the button to get assigned their species
- Pressing the undo button goes back to re-draft the player who last drafted
- After all players have drafted, play order is shuffled

3. Turn tracking

- The play order is displayed on the row of lights
- The current player is shown by the button light
- When the current player is done with their turn, they hit the button and it advances to the next plyaer
- If the current player passes, they double-tap the arcade button. Their turn order light is dimmed and they are removed from the play order.
- After all players pass, the pass order is used to set the new play order and all players are reset
- The undo button reverts whatever the last button press did

There is also a battery check mode, which I use when charging the tracker. This is partially because I wired
the tracker in such a way that I can't charge the battery unless it's turned on. (The battery itself is directly
switched.)

Pressing the undo button 3 times quickly will toggle battery check mode. In this mode, one light will remain on
to show the voltage (and charge state) state of the battery:

- Green >4.0V
- Yellow 3.6V - 4.0V
- Orange 3.4V - 3.6V
- Red <3.4V

Pressing the undo button 3 times again will return to the previous mode.

## This Repo

The [tracker.ino](/tracker/tracker.ino) file is an Arduino sketch. I used an ATmega 328P based Arduino, specifically an Adafruit Feather 328P.

Also included is the source Fusion 360 file for the enclosure, and individual 3MF files for the three parts. The parts are:

- Main body
- Lid
- Indicators

The main body and lid are pretty straightforward - I printed them with PLA using 0.2mm layer height.

The indicators need to be printed in 2 colors - an opaque color for the bottom where the NeoPixels are mounted,
and a translucent color for the indicators themselves.

![Detail of the indicator slicing](./indicators.png)

## Parts

Parts used were:

- [Adafruit Feather 328P](https://www.adafruit.com/product/3458)
- 7 x [Adafruit NeoPixel Mini Button](https://www.adafruit.com/product/1612)
- [Arcade Button - 30mm Translucent Clear](https://www.adafruit.com/product/471)
  - This was modified to contain a NeoPixel per [this guide](https://learn.adafruit.com/neopixel-arcade-button)
- [16mm Panel Mount Momentary Pushbutton - Red](https://www.adafruit.com/product/1445)
- [Lithium Ion Polymer Battery - 3.7v 2500mAh](https://www.adafruit.com/product/328)
- [SW-R3-1A-A-1-0 Subminiature Rocker Switch](https://www.digikey.com/short/hp38423t)
- 4 x M2x5 self-tapping screws to mount the Feather
- 6 x M3x8 countersunk screws to attach the top
- 6 x M3 threaded inserts

## Oh hi

I might get around to documenting more, but until then, get in touch with me if you have any questions!
micah@xlmpq.com
