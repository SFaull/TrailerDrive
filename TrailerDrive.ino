#define SW0  2 // Mode change switch at pin 2
#define SW1  3 // Manual select switch at pin 3

#define RELAY_0 6   // Relay channel 0 at pin 6
#define RELAY_1 7   // Relay channel 1 at pin 7
#define RELAY_2 8   // Relay channel 2 at pin 8
#define RELAY_3 9   // Relay channel 3 at pin 9
#define RELAY_4 10  // Relay channel 4 at pin 10
#define RELAY_5 11  // Relay channel 5 at pin 11
#define RELAY_6 12  // Relay channel 6 at pin 12

unsigned long interval = 3000;   
unsigned long previousMillis = 0;

int count = 0;
int SW0_lastVal = 0;
int SW0_val = 0;
int SW1_lastVal = 0;
int SW1_val = 0;
bool MANUAL = false;
bool CYCLE = false;
int cycleCount = 0;

void setup() 
{
  Serial.begin(9600);    // make sure your monitor baud rate matches this
  
  pinMode(SW0,INPUT);
  pinMode(SW1,INPUT);
  
  pinMode(RELAY_0, OUTPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);

  setLow(); // Initially all outputs OFF
  Serial.println("Ready...");
  
  // put into mode 0
    Serial.print("MODE: ");
    Serial.println(count);
    CYCLE = false;
    MANUAL = false;
    setHigh();
  


}

void loop() 
{
  unsigned long currentMillis = millis();       // Get runtime
  SW0_lastVal = SW0_val;
  SW1_lastVal = SW1_val;
  SW0_val = digitalRead(SW0);
  SW1_val = digitalRead(SW1);
  
  if( (SW0_val == 1) && (SW0_lastVal == 0) )  // when  mode button is pushed
  {
    if (count < 2)
      count = count + 1;
    else
      count = 0;
      
    Serial.print("MODE: ");
    Serial.println(count);

    switch (count)
    {
      case 0: // This mode turns all outputs on
        CYCLE = false;
        MANUAL = false;
        setHigh();
      break;
  
      case 1: // This mode cycles outputs
        setLow();
        cycleCount = 0;
        CYCLE = true;
        MANUAL = false;
      break;
  
      case 2: // This mode allows manual selection of output
        cycleCount = 0;
        CYCLE = false;
        MANUAL = true;
        setLow();
      break;
  
      default:
        setLow();
        CYCLE = false;
        MANUAL = false;
        Serial.println("UNKNOWN MODE");
      break;
    }

  }

if (CYCLE)
{
  if (currentMillis - previousMillis >= interval) // if the interval is up...
  {
    runCycle(cycleCount);
    previousMillis = currentMillis;     // save the last time LED was blinked
    if (cycleCount < 6)
      cycleCount = cycleCount + 1;
    else
      cycleCount = 0;
  }
}

if (MANUAL)
{
  if( (SW1_val == 1) && (SW1_lastVal == 0) )  // when  selection button is pushed
  {
    runCycle(cycleCount);
    if (cycleCount < 6)
      cycleCount = cycleCount + 1;
    else
      cycleCount = 0;
  }
}

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

void runCycle(int cc)
{
  setLow();
  int tmp = RELAY_0 + cc;
  digitalWrite(tmp, LOW); // Active low
  Serial.print("Relay ");
  Serial.print(cc);
  Serial.println(" ON");
}

