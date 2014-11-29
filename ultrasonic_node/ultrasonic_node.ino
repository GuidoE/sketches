#include <RFM69.h>
#include <SPI.h>
#include <Ultrasonic.h>
#define NODEID      99
#define NETWORKID   100
#define GATEWAYID   1
#define FREQUENCY   RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "1234098712340987" //has to be same 16 characters/bytes on all nodes, not more not less!
#define LED         9
#define SERIAL_BAUD 115200
#define ACK_TIME    30  // # of ms to wait for an ack
#include <math.h>

int photoValue;
int temp;
int TRANSMITPERIOD = 300; //transmit a packet to gateway so often (in ms)
byte sendSize=0;
boolean requestACK = false;
RFM69 radio;
Ultrasonic ultrasonic(5, 6, 20000);

//defines data struct
typedef struct {		
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  int         inches;   //ultrasonic data
  int         photo; //photoresistor data
  int         temp; //theremistor data
} Payload;

//instantiates data struct
Payload theData;


double Thermistor(int RawADC) {
 double Temp;
 RawADC = RawADC * 3.3 / 4.772956;//converts 3.3v analog input into 5.0v
 Temp = log(10000.0*((1024.0/RawADC-1))); 
//         =log(10000.0/(1024.0/RawADC-1)) // for pull-up configuration
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
 Temp = Temp - 273.15;            // Convert Kelvin to Celcius
 Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
 return Temp;
}

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
  
  //pinMode(4, OUTPUT); // VCC pin
  //pinMode(7, OUTPUT); // GND ping
  //digitalWrite(4, HIGH); // VCC +5V mode  
  //digitalWrite(7, LOW);  // GND mode
}

long lastPeriod = -1;
byte ackCount=0;

void loop()
{
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
    photoValue = analogRead(A4);
    temp = int(Thermistor(analogRead(A5)));
    
    int inches;
    inches = ultrasonic.Ranging(INC);
    Serial.print(inches); // CM or INC
    Serial.print(" inches " );
    Serial.print(photoValue);
    Serial.print(" light ");
    Serial.print(temp);
    Serial.println(" F");
    theData.nodeId = NODEID;
    theData.uptime = millis();
    theData.inches = inches;
    theData.photo = photoValue;
    theData.temp = temp;
    Serial.print("Sending struct (");
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
