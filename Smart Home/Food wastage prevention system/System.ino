#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <pt.h>

#define FIREBASE_HOST "fwbs-11340.firebaseio.com"
#define FIREBASE_HOST1 "data-from-nodemcu.firebaseio.com"
#define FIREBASE_AUTH1 "ZrF3NY738uWl4YaXvnHryhIPsYO7UlDx86Pnah61"
#define FIREBASE_AUTH "AeRPoqyoWbbivLsS2gcJw1Z8uo3quz3cfPEmWQbe"

// set the Wifi Parameters
const char *ssid =  "lala";     // replace with your wifi ssid and wpa2 key
const char *pass =  "lalaland";

WiFiClient client; 

//Declare Thread varaibles
static struct pt pt1, pt2,pt3;

// Declare Force Sensor Pins
String result;
float val;
int forcePin = A0;
int forceReading=0;

//Declare Touch Sensor Paramters
int sec=30;
float i=sec;
int tp= 14;//D5
int detect;

//Declare Led Pins
int led1= 4;  //D2 (food is rotten)
int led2= 16; //D0 (food is fresh)
int led3= 5;  //D1 (food is about to get rotton)
 
void setup() 
{
  Serial.begin(9600);
       delay(10);
               
       Serial.println("Connecting to ");
       Serial.println(ssid); 
 
       WiFi.begin(ssid, pass); 
       while (WiFi.status() != WL_CONNECTED) 
          {
            delay(500);
            Serial.print(".");
          }
      Serial.println("");
      Serial.println("WiFi connected"); 
      Serial.println("NodeMCU IP Address:  ");
      Serial.println(WiFi.localIP());
    
      //Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
      //Firebase.begin(FIREBASE_HOST1,FIREBASE_AUTH1);
      
  //Set Led Pins as Output
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);

  //initialise the two protothread variables
  PT_INIT(&pt1); 
  PT_INIT(&pt2);
  PT_INIT(&pt3);
}

void status1(float j)
{
  float per =(j/sec)*100;
  //Serial.println(per);
  if(per <= 20.00)
  {
    digitalWrite(led1,HIGH);
    digitalWrite(led2,LOW);
    digitalWrite(led3,LOW);
    
  
  }
  if(per >= 21.00 && per <= 59.00)
  {
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);
    digitalWrite(led3,HIGH);
  
  }
  if(per >= 60)
  {
   digitalWrite(led1,LOW);
   digitalWrite(led2,HIGH);
   digitalWrite(led3,LOW);
    
  }
}

void transmitting()
{
  Firebase.begin(FIREBASE_HOST1,FIREBASE_AUTH1);
  forceReading = analogRead(forcePin);
  val=float(forceReading);
  if(val<20)
  {
    val=1;
  }
  else
  {
    val=0;
  }
  Serial.print("Analog reading = ");
  Serial.println(forceReading);
  Firebase.setFloat("Thr",val);
  Firebase.setFloat("Timer",i);  
    if (Firebase.failed()) 
    {
      Serial.print("pushing /logs failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(500);
  
}

void receiving()
{
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
  result=Firebase.getString("gg/1");
  Serial.println(result);
  if (Firebase.failed()) 
    {
      Serial.print("pushing /logs failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(500);
}

void Timer()
{
  detect=digitalRead(tp);
  delay(10);
  if(detect==HIGH)
  {
  status1(i);
  }

  else
  {
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);
    digitalWrite(led3,LOW);
  }

 delay(1);
 i--;
}

static int  Thread1(struct pt *pt, int interval)
{
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) { // never stop 
    /* each time the function is called the second boolean
    *  argument "millis() - timestamp > interval" is re-evaluated
    *  and if false the function exits after that. */
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis(); // take a new timestamp
    transmitting();
  }
  PT_END(pt);
}

static int Thread2(struct pt *pt, int interval)
{
   static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) 
  {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis();
     receiving();
  }
  PT_END(pt);
}

static int Thread3(struct pt *pt, int interval)
{
   static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) 
  {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis();
    Timer();
  }
  PT_END(pt);
}




void loop() 
{
  Thread1(&pt1, 1400); // schedule the two protothreads
  Thread2(&pt2, 1500); // by calling them infinitely
  Thread3(&pt3, 1600);
}
