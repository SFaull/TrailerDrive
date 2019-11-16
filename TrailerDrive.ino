#include "FastLED.h"


#define NUM_OUTPUTS 6
#define CYCLE_TIMEOUT 3000 // Time between switching outputs (in seconds)
#define INPUT_READ_TIMEOUT     50   //check for button pressed every 50ms

#define LED_DATA_PIN 10
#define PB0  A0  // Mode change switch at pin A0
#define PB1  A1  // Manual select switch at pin A1
#define LED0 A2  // LED 0 at pin A2
#define LED1 A3  // LED 1 at pin A3

#define RELAY_0 2    // Relay channel 0 at pin 2
#define RELAY_1 3    // Relay channel 1 at pin 3
#define RELAY_2 4    // Relay channel 2 at pin 4
#define RELAY_3 5    // Relay channel 3 at pin 5
#define RELAY_4 6    // Relay channel 4 at pin 6
#define RELAY_5 7    // Relay channel 5 at pin 7
#define RELAY_6 8    // Relay channel 6 at pin 8 (CURRENTLY UNUSED)
#define RELAY_7 9    // Relay channel 7 at pin 9 (CURRENTLY UNUSED)

#define MAX_BRIGHTNESS 255

unsigned long readInputTimer  = 0,
              cycleTimer = 0;

bool  PB0_pressed, 
      PB1_pressed = false;

enum {ALL, CYCLE, MANUAL} STATE;  // stores current mode of operation
int cycleCount = 0; // stores the current output when cycling

CRGB leds[NUM_OUTPUTS+2]; // we have 2 more leds than outputs
CRGB wireColour[NUM_OUTPUTS] = 
{
  CRGB::Yellow,
  CRGB::White,
  CRGB::Red,
  //CRGB::SaddleBrown,
  CRGB::Orange,
  CRGB::Green,
  CRGB::Blue
};

void setup() 
{
  // Start serial
  Serial.begin(9600);    // make sure your monitor baud rate matches this
  // Inputs
  pinMode(PB0, INPUT_PULLUP);  // Enables the internal pull-up resistor
  pinMode(PB1, INPUT_PULLUP);  // Enables the internal pull-up resistor
  //Outputs
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(RELAY_0, OUTPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);
  pinMode(RELAY_7, OUTPUT);

  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_OUTPUTS+2).setCorrection( TypicalLEDStrip );

  // Initially set mode to display all outputs;
  STATE = CYCLE;
  stateInit();

  FastLED.setBrightness(MAX_BRIGHTNESS);

  setTimer(&readInputTimer);  // reset timer
  setTimer(&cycleTimer);  // reset timer
}

void loop() 
{
  readInputs();
  setState();
  stateProcess();
}

void readInputs(void)
{
  static bool PB0_state, 
              last_PB0_state, 
              PB1_state, 
              last_PB1_state = false; // Remembers the current and previous button states
              
  /* Periodically read the inputs */
  if (timerExpired(readInputTimer, INPUT_READ_TIMEOUT)) // check for button press periodically
  {
    setTimer(&readInputTimer);  // reset timer

    // store the previous state
    last_PB0_state = PB0_state;
    last_PB1_state = PB1_state;
    
    // read pins
    PB0_state = !digitalRead(PB0); // active low
    PB1_state = !digitalRead(PB1); // active low
  
    if (!PB0_state && last_PB0_state) // on a falling edge we register a button press
    {
      PB0_pressed = true;
      Serial.println("PB0 pushed");
    }
  
    if (!PB1_state && last_PB1_state) // on a falling edge we register a button press
    {
      PB1_pressed = true;
      Serial.println("PB1 pushed");
    }
  }
}

void setLow() // All outputs off (active low)
{  
  for (int i=0; i<NUM_OUTPUTS+2; i++)  // turn all LEDs off
  {
      digitalWrite(RELAY_0 + i, HIGH);
      leds[i] = CRGB::Black;
  }
  FastLED.show();

  Serial.println("All outpts OFF");
}

void setHigh() // All outputs on (active low)
{
  for(int i=0; i<NUM_OUTPUTS; i++) // turn all LEDs on
  {
    digitalWrite(RELAY_0 + i, LOW);
    leds[i+1] = wireColour[i];
  }
  FastLED.show();

  Serial.println("All outpts ON");
}

void LED(int pin)
{
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, LOW);

  if (pin == 1)
    digitalWrite(LED0, HIGH);
  else if (pin == 2)
    digitalWrite(LED1, HIGH);
}

void runCycle(int cc)
{
  setLow();
  
  if(cc < NUM_OUTPUTS)
  {
    int tmp = RELAY_0 + cc;
    
    leds[cc] = wireColour[cc];
    leds[cc+1] = wireColour[cc];
    leds[cc+2] = wireColour[cc];
    digitalWrite(tmp, LOW); // Active low
    FastLED.show();
    
    Serial.print("Relay ");
    Serial.print(cc);
    Serial.println(" ON");
  }
  else
  {
    // special case for final state
    leds[1] = wireColour[1];
    leds[2] = wireColour[1];
    leds[4] = wireColour[3];
    leds[5] = wireColour[3];
    digitalWrite(RELAY_1, LOW); // Active low (left side)
    digitalWrite(RELAY_3, LOW); // Active low (right side)
    FastLED.show();
    
    Serial.print("Relay ");
    Serial.print(1);
    Serial.println(" ON");

    Serial.print("Relay ");
    Serial.print(3);
    Serial.println(" ON");
  }
}

void setState()
{
  if (PB0_pressed)
  {
    PB0_pressed = false;

    if (STATE < 2)
      STATE = STATE + 1;
    else
      STATE = 0;
 
    stateInit();
  }
}

void stateInit(void)
{
  Serial.print("State: ");
  Serial.println(STATE);
 
  switch (STATE)
  {
    case ALL: // This mode turns all outputs on
      setLow();
      setHigh();
      LED(1);
    break;

    case CYCLE: // This mode cycles outputs
      setLow();
      cycleCount = 0;
      LED(2);
      //runCycle(cycleCount);
    break;

    case MANUAL: // This mode allows manual selection of output
      cycleCount = 0;
      setLow();
      LED(0);
      //runCycle(cycleCount);
    break;

    default:
      setLow();
      Serial.println("UNKNOWN MODE");
    break;
  }
}

void stateProcess(void)
{
  switch (STATE)
  {
    case ALL: // This mode turns all outputs on
      // do nothing
    break;

    case CYCLE: // This mode cycles outputs
      if (timerExpired(cycleTimer, CYCLE_TIMEOUT)) // check for button press periodically
      {
        setTimer(&cycleTimer);  // reset timer
        runCycle(cycleCount);
        
        if (cycleCount < NUM_OUTPUTS)
          cycleCount = cycleCount + 1;
        else
          cycleCount = 0;
      }
    break;

    case MANUAL: // This mode allows manual selection of output
      if(PB1_pressed)  // when  selection button is pushed
      {
        PB1_pressed = false;
        runCycle(cycleCount);
        
        if (cycleCount < NUM_OUTPUTS)
          cycleCount = cycleCount + 1;
        else
          cycleCount = 0;        
      }
    break;

    default:
    // do nothing
    break;
  }
}

/* pass this function a pointer to an unsigned long to store the start time for the timer */
void setTimer(unsigned long *startTime)
{
  *startTime = millis();  // store the current time
}

/* call this function and pass it the variable which stores the timer start time and the desired expiry time 
   returns true fi timer has expired */
bool timerExpired(unsigned long startTime, unsigned long expiryTime)
{
  if ( (millis() - startTime) >= expiryTime )
    return true;
  else
    return false;
}
