# PulseMIDI-LED
Project to combine MIDI-LED, JiggleJam MIDI Interface, Heartbeat Sensor

Microcontrollers:
TB: BareConductive TouchBoard (Arduino Leonardo)
HB: Arduino Nano v3.0
AD: AD8232-EvalZ EKG Sensor Breakout
UN: unnecessary extra microcontroller

HB: receives pulse data from AD, blinks onboard LED, sends High/Low to a digital pin for transmission to TB (High for Pulse)
TD: receives High/Low on digital pin 13, sends MIDI note to Ableton to create sounds; simultaneously listens to it's own electrode to send a different MIDI note when that electrode is touched; also blinks (different or same?) LED for each electrode touch
AD: sends pulse data to HB
UN: when switch is flipped, do a rainbow sweep on 24-LED NeoPixel Ring.

Upgrades:
Combine HB & TB pulse LED blink to the same LED (consolidate blink code to TB?)
UN receives a MIDI signal from Ableton to sweep the rainbow, rather than a physical switch
Remove unnecessary UN - replace with TB?
