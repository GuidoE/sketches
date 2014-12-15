#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <ArduinoJson.h>

#define NODEID      1
#define NETWORKID   100
#define FREQUENCY   RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "1234098712340987" //has to be same 16 characters/bytes on all nodes, not more not less!
#define LED         9
#define SERIAL_BAUD 115200
#define ACK_TIME    30  // # of ms to wait for an ack
#define RELAY_ON 1
#define RELAY_OFF 0
#define RELAY_1  3

RFM69 radio;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

typedef struct {		
  char      data[54];
} Payload;
Payload theData;
  
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.setHighPower(); //uncomment only for RFM69HW!
  radio.encrypt(KEY);
  radio.promiscuous(promiscuousMode);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  if (flash.initialize())
    Serial.println("SPI Flash Init OK!");
  else
    Serial.println("SPI Flash Init FAIL! (is chip present?)");
  // Set pin as output.
  pinMode(RELAY_1, OUTPUT);
 
  // Initialize relay one as off so that on reset it would be off by default
  digitalWrite(RELAY_1, RELAY_OFF);
}

byte ackCount=0;
void loop() {
  //process any serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input == 'r') //d=dump all register values
      radio.readAllRegs();
    if (input == 'E') //E=enable encryption
      radio.encrypt(KEY);
    if (input == 'e') //e=disable encryption
      radio.encrypt(null);
    if (input == 'p')
    {
      promiscuousMode = !promiscuousMode;
      radio.promiscuous(promiscuousMode);
      Serial.print("Promiscuous mode ");Serial.println(promiscuousMode ? "on" : "off");
    }
    
    if (input == 'i')
    {
      Serial.print("DeviceID: ");
      word jedecid = flash.readDeviceId();
      Serial.println(jedecid, HEX);
    }
    
    if (input == 'o') {
      // Turn on and wait 3 seconds.
      digitalWrite(RELAY_1, RELAY_ON);
    }
    
    if (input == 'f') {
      // Turn on and wait 3 seconds.
      digitalWrite(RELAY_1, RELAY_OFF);
    }
    
    if (input == 't')
    {
      byte temperature =  radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
      byte fTemp = 1.8 * temperature + 32; // 9/5=1.8
      Serial.print( "Radio Temp is ");
      Serial.print(temperature);
      Serial.print("C, ");
      Serial.print(fTemp); //converting to F loses some resolution, obvious when C is on edge between 2 values (ie 26C=78F, 27C=80F)
      Serial.println('F');
    }
  }

  if (radio.receiveDone())
  {    
    StaticJsonBuffer<200> jsonBuffer;
    /*if (promiscuousMode)
	{
      Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
    }*/
    theData = *(Payload*)radio.DATA;
    JsonObject& root = jsonBuffer.parseObject(theData.data);

    if (!root.success())
    {
      Serial.print("Radio Data Length ");
      Serial.println(radio.DATALEN);
      Serial.print("Object Length ");
      Serial.print(sizeof(theData));
      Serial.print("Data object ");
      Serial.println(theData.data);
      Serial.println(" parseObject() failed");
      return;
    }
    int relayValue;
    relayValue = digitalRead(RELAY_1);
    
    root["signal"] = radio.readRSSI();
    root["id"] = radio.SENDERID;
    root["uptime"] = theData.uptime;
    root["relay"] = relayValue;
    
    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      //Serial.print(" - ACK sent.");

      // When a node requests an ACK, respond to the ACK
      // and also send a packet requesting an ACK (every 3rd one only)
      // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
      if (ackCount++%3==0)
      {
        //Serial.print(" Pinging node ");
        //Serial.print(theNodeID);
        //Serial.print(" - ACK...");
        delay(3); //need this when sending right after reception .. ?
        if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0)){  // 0 = only 1 attempt, no retries
          //Serial.print("ok!");
          root["ack"] = "ok!";
        }
        else {
          //Serial.print("nothing");
          root["ack"] = "nothing";
        }
      }
    }
    root.printTo(Serial);
    Serial.println();
    Blink(LED,3);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
