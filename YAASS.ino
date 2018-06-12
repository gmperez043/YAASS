#include <MIDI.h>

// Arduino board pin stuff
#define MILIS_PER_STEP (1000.0 / 120.0) * 15.0 // Tempo, only 120 bpm. TODO: Get tempo from MIDI IN.

// AnalogRead(0) and DigitalRead(0) return different values.
// Analog and digital pins can be the same number.
const byte LedPins[8] = {6, 7, 8, 9, 10, 11, 12, 13};
const byte KnobPins[8] = {7, 6, 5, 4, 3, 2, 1, 0};

// Sequence length
const byte max_steps = 8;

// Created and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

void setup()
{
  MIDI.begin(MIDI_CHANNEL_OMNI); // Listen to all incoming messages

  MIDI.turnThruOff();
  MIDI.setHandleStart(OnStart);
  MIDI.setHandleStop(OnStop);
  MIDI.setHandleNoteOn(OnNoteOn);

  Serial.begin(115200); // Needed for the serial to MIDI converter software

  // Only need to set pinMode on digital pins.
  // Dont set analog pins mode, it will set digital pins instead!!!
  for (byte i = 0; i < max_steps; i++)
    pinMode(LedPins[i], OUTPUT);

  TestLeds();
}

// SEQUENCER STUFF
unsigned long nextMillis = 0;
unsigned long currentMillis = 0;
unsigned long noteOffMillis = 0;
byte currentNote = 0;
byte baseNote = 24; // C - 0
bool isNoteOn = false;
bool running = false;
byte actualStep;
float noteLength = .25f; // 0 to 1, step percentage

void loop()
{
  MIDI.read();

  if (!running)
    return;

  currentMillis = millis();

  if (currentMillis >= nextMillis)
  {
    // Advance sequencer
    actualStep++;
    if (actualStep >= max_steps)
      actualStep = 0;

    noteOffMillis = nextMillis + (MILIS_PER_STEP * noteLength);
    nextMillis += MILIS_PER_STEP;

    // Read input
    currentNote = analogRead(KnobPins[actualStep]) / 42; // 1024 / 42 = 24. 2 scale range
    currentNote = baseNote + currentNote;

    // Send note
    MIDI.sendNoteOn(currentNote, 127, 1);
    isNoteOn = true;

    // Update interface
    UpdateLeds(actualStep);
  }
  else if (isNoteOn && currentMillis >= noteOffMillis)
  {
    MIDI.sendNoteOff(currentNote, 127, 1);
    isNoteOn = false;
  }
}

void OnStart()
{
  actualStep = 255; // ++ overflows to 0.
  running = true;
  nextMillis = millis();
}

void OnStop()
{
  running = false;
  MIDI.sendNoteOff(currentNote, 127, 1);
}

void OnNoteOn(byte channel, byte note, byte velocity)
{
  baseNote = note;
}

void TestLeds()
{
  for (byte i = 0; i < max_steps; i++)
  {
    digitalWrite(LedPins[i], HIGH);
    delay(100);
  }

  for (int i = max_steps - 1; i >= 0; i--) // If change this to byte does not work...
  {
    digitalWrite(LedPins[i], LOW);
    delay(100);
  }
}

void UpdateLeds(byte ledOn)
{
  for (byte i = 0; i < max_steps; i++)
    digitalWrite(LedPins[i], LOW);

  digitalWrite(LedPins[ledOn], HIGH);
}