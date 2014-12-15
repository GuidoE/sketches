#include <RFM69.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include <ArduinoJson.h>

#define NODEID      98
#define NETWORKID   100
#define GATEWAYID   1
#define FREQUENCY   RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "1234098712340987" //has to be same 16 characters/bytes on all nodes, not more not less!
#define LED         9
#define GAS      3
#define SOIL      5
#define SERIAL_BAUD 115200
#define ACK_TIME    30  // # of ms to wait for an ack
#define ONE_WIRE_BUS 8 // Data wire is plugged into port 2 on the Arduino

int photoValue;
int gas;
int moisture;

int TRANSMITPERIOD = 300; //transmit a packet to gateway so often (in ms)
byte sendSize=0;
boolean requestACK = false;
RFM69 radio;


//defines data struct
typedef struct {		
  unsigned long uptime; //uptime in ms
  char  data[48];
} Payload;

//instantiates data struct
Payload theData;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);
  
  //Sets up RFM
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.setHighPower(); //uncomment only for RFM69HW!
  radio.encrypt(KEY);
  
  //creates data buffer
  char buff[50];
  
  //Logs transmission status
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);  
  pinMode(7, INPUT);
  
  // Start up the OneWire library
  sensors.begin();
  //pinMode(4, OUTPUT); // VCC pin
  //pinMode(7, OUTPUT); // GND ping
  //digitalWrite(4, HIGH); // VCC +5V mode  
  //digitalWrite(7, LOW);  // GND mode
}

long lastPeriod = -1;
byte ackCount=0;

void loop()
{
  sensors.requestTemperatures(); // Send the command to get temperatures
  int currPeriod = millis()/TRANSMITPERIOD;
  //check for any received packets
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
    }
    Blink(LED,5);
    Serial.println();
  }
  if (currPeriod != lastPeriod)
  {
    //read the input from A4 and store it in a variable
    photoValue = map(analogRead(A4), 0, 1023, 0, 100);
    int tempValue = DallasTemperature::toFahrenheit(sensors.getTempCByIndex(0));
    
    int sensorValue = analogRead(A2);
    gas = map(sensorValue, 0, 1023, 0, 100);
    moisture = digitalRead(7);
    
    if (gas > 25) Blink(GAS,1000);
    
    if (moisture == 1) Blink(SOIL,1000);
    
    //Instantiates JSON buffer
    StaticJsonBuffer<48> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    
    root["gas"] = gas;
    root["light"] = photoValue;
    root["soil"] = moisture;
    root["temp"] = tempValue;
    
    Serial.print("JSON buffer size ");
    Serial.println(sizeof(jsonBuffer));
    root.printTo(theData.data, sizeof(theData.data));

    Serial.print("Sending JSON (");
    Serial.print(sizeof(theData));
    Serial.print(" bytes) ... ");
    if (radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData)))
      Serial.print(" ok!");
    else Serial.print(" nothing...");
    Serial.println();
   
    Blink(LED,3);
    lastPeriod=currPeriod;
    delay(1000);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
