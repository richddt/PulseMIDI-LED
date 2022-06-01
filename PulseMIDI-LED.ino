#include <FastLED.h>
#include <MPR121.h>

#define PIN_HEART 13
#define PIN_BEAT_LED 11
#define PIN_RING_LED 12
#define NUM_ELECTRODES 12
#define RING_LED_COUNT 24
#define HEART_NOTE 47
#define HEART_NOTE_ON_MILLIS 100

// piano notes from C3 to B3 in semitones - you can replace these with your own values
// for a custom note scale - http://newt.phys.unsw.edu.au/jw/notes.html has an excellent
// reference to convert musical notes to MIDI note numbers
const uint8_t notes[NUM_ELECTRODES] = {59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48};
const uint8_t channel = 0; // default channel is 0

bool heartNoteOn = false;
bool crazyModeOn = false;
bool beatFadingOut = false;
uint8_t ringLedInitialHue = 0;

CRGB beatLed[1];
CRGB ringLed[RING_LED_COUNT];

void setup() {
  Serial.begin(9600);

  MPR121.begin(0x5C);
  MPR121.setInterruptPin(4);
  MPR121.updateTouchData();

  FastLED.addLeds<WS2811, PIN_BEAT_LED>(beatLed, 1);
  FastLED.addLeds<NEOPIXEL, PIN_RING_LED>(ringLed, RING_LED_COUNT);

  pinMode(PIN_HEART, INPUT);
}

void loop() {
  // Stop playing the heart note if it's time, or start playing it if the heart pin is HIGH
  if (heartNoteOn) {
    EVERY_N_MILLISECONDS(HEART_NOTE_ON_MILLIS) {
      heartNoteOff();
    }
  }
  else if (digitalRead(PIN_HEART) == HIGH) {
    heartNoteOn = true;
    Serial.println(F("Heart on"));
    beatLedOn();
    noteOn(channel, HEART_NOTE, 127);
  }

  // Handle any changes to the touch sensors
  if (MPR121.touchStatusChanged()) {
    MPR121.updateTouchData();
    for (int i = 0; i < NUM_ELECTRODES; i++) {
      if (MPR121.isNewTouch(i)) {
        // if we have a new touch, turn on the LED and send a "note on" message
        Serial.print("Touch ");
        Serial.println(i);
        beatLedOn();
        noteOn(channel, notes[i], 127); // maximum velocity
      }
      else if (MPR121.isNewRelease(i)) {
        // if we have a new release, turn off the LED and send a "note off" message
        beatLedOff();
        noteOff(channel, notes[i], 127); // maximum velocity
        Serial.print("Release ");
        Serial.println(i);
      }
    }

    // flush USB buffer to ensure all notes are sent
    MIDIUSB.flush();
  }

  // Check for a MIDI note to enable/disable crazy mode
  if (MIDIUSB.peek().type != MIDI_EVENT_NONE.type) {
    MIDIEvent event = MIDIUSB.read();

    Serial.print(F("MIDI in! type=0x"));
    Serial.print(event.type, HEX);
    Serial.print(F(" data=0x"));
    Serial.print(event.m1, HEX);
    Serial.print(F(" 0x"));
    Serial.print(event.m2, HEX);
    Serial.print(F(" 0x"));
    Serial.print(event.m3, HEX);
    Serial.println();

    // TODO: is the type actually filled in?
    // TODO: should we filter on a particular channel or for a particular pitch?
    if (event.type == 0x08) { // Note off
      crazyModeOn = false;
      ringLedOff();
    }
    else if (event.type == 0x09) { // Note on
      crazyModeOn = true;
      ringLedInitialHue = 0;
    }
  }

  // When in crazy mode or the beat LED is fading out, animate.
  if (crazyModeOn || beatFadingOut) {
    EVERY_N_MILLISECONDS(10) {
      if (crazyModeOn) {
        ringLedAnimate();
      }
      if (beatFadingOut) {
        beatLedFade();
      }
      FastLED.show();
    }
  }
}

void noteOn(uint8_t channel, uint8_t pitch, uint8_t velocity) {
  MIDIUSB.write({0x09, 0x90 | (channel & 0x0F), pitch, velocity});
}

void noteOff(uint8_t channel, uint8_t pitch, uint8_t velocity) {
  MIDIUSB.write({0x08, 0x80 | (channel & 0x0F), pitch, velocity});
}

void heartNoteOff() {
  beatLedOff();
  noteOff(channel, HEART_NOTE, 0);
  heartNoteOn = false;
  Serial.println(F("Heart off"));
}

void beatLedOn() {
  beatFadingOut = false;
  beatLed[0] = CRGB(127, 127, 127);
  FastLED.show();
}

void beatLedOff() {
  beatFadingOut = true;
}

void beatLedFade() {
  fadeToBlackBy(beatLed, 1, 4);
}

void ringLedAnimate() {
  fill_rainbow(ringLed, RING_LED_COUNT, ringLedInitialHue++, 255 / RING_LED_COUNT);
}

void ringLedOff() {
  fill_solid(ringLed, RING_LED_COUNT, CRGB::Black);
  FastLED.show();
}
