#define INTERVAL 3000 // Time between switching outputs (in seconds)

#define PB0  2 // Mode change switch at pin 2
#define PB1  3 // Manual select switch at pin 3

#define RELAY_0 6   // Relay channel 0 at pin 6
#define RELAY_1 7   // Relay channel 1 at pin 7
#define RELAY_2 8   // Relay channel 2 at pin 8
#define RELAY_3 9   // Relay channel 3 at pin 9
#define RELAY_4 10  // Relay channel 4 at pin 10
#define RELAY_5 11  // Relay channel 5 at pin 11
#define RELAY_6 12  // Relay channel 6 at pin 12

#define LED0 A0  // LED 0 at pin A0
#define LED1 A1  // LED 1 at pin A1
#define LED2 A2  // LED 2 at pin A2

unsigned long currentMillis,      // runtime in ms
              previousMillis = 0; // used to store time checkpoints

bool  PB0_pressed, 
      PB1_pressed = false;

enum {ALL, CYCLE, MANUAL} STATE;  // stores current mode of operation
int cycleCount = 0; // stores the current output when cycling

void setup() 
{
  // Start serial
  Serial.begin(9600);    // make sure your monitor baud rate matches this
  // Inputs
  pinMode(PB0, INPUT_PULLUP);  // Enables the internal pull-up resistor
  pinMode(PB1, INPUT_PULLUP);  // Enables the internal pull-up resistor
  //Outputs
  pinMode(RELAY_0, OUTPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);

  // Initially set mode to display all outputs;
  STATE = ALL;
  stateInit();
}

void loop() 
{
  currentMillis = millis();       // Get runtime
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
  delay(10);
}

void setLow() // All outputs off (active low)
{
  digitalWrite(RELAY_0, HIGH);
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
  digitalWrite(RELAY_5, HIGH);
  digitalWrite(RELAY_6, HIGH);

  Serial.println("All outpts OFF");
}

void setHigh() // All outputs on (active low)
{
  digitalWrite(RELAY_0, LOW);
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW);
  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);
  digitalWrite(RELAY_5, LOW);
  digitalWrite(RELAY_6, LOW);

  Serial.println("All outpts ON");
}

void LED(int pin)
{
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  
  if (pin == 0)
    digitalWrite(LED0, HIGH);
  else if (pin == 1)
    digitalWrite(LED1, HIGH);
  else if (pin == 2)
    digitalWrite(LED2, HIGH);
}

void runCycle(int cc)
{
  setLow();
  int tmp = RELAY_0 + cc;
  digitalWrite(tmp, LOW); // Active low
  Serial.print("Relay ");
  Serial.print(cc);
  Serial.println(" ON");
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
      setHigh();
      LED(0);
    break;

    case CYCLE: // This mode cycles outputs
      setLow();
      cycleCount = 0;
      LED(1);
    break;

    case MANUAL: // This mode allows manual selection of output
      cycleCount = 0;
      setLow();
      LED(2);
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
      if (currentMillis - previousMillis >= INTERVAL) // if the INTERVAL is up...
      {
        runCycle(cycleCount);
        previousMillis = currentMillis;     // save current time
        if (cycleCount < 6)
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
        if (cycleCount < 6)
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

