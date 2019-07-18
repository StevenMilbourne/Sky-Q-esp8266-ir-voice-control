#define FFRWMINUTE 1800
#define FFRWSECOND 180
#define SEARCHDELAY 200
#define SAMEBUTTONDELAY 800

#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRtimer.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>

const char* ssid = "";
const char* password = "";
MDNSResponder mdns;
ESP8266WebServer server(1066);

/* 
 *  Sky Q Remote IR Codes
 */
 
int Sky  = 0xC0081A80;
int Power = 0xC0081A0C;
int Search = 0xC0081A7E;
int Rewind  = 0xC0081A3D;
int PlayPause  = 0xC0081A3E;
int Forward = 0xC0081A28;
int Up  = 0xC0081A58;
int Down  = 0xC0081A59;
int Left  = 0xC0081A5A;
int Right = 0xC0081A5B;
int Select  = 0xC0081A5C;
int Back = 0xC0081A83;
int Home  = 0xC0081ACC;
int Information = 0xC0081ACB;
int PageUP  = 0xC0081A20;
int PageDOWN  = 0xC0081A21;
int Record  = 0xC0081A40;
int ButtonRED = 0xC0081A6D;
int ButtonGREEN = 0xC0081A6E;
int ButtonYELLOW  = 0xC0081A6F;
int ButtonBLUE  = 0xC0081A70;
int HELP  = 0xC0081A81;
int Button[11] = {
  /* Button 0 */ 0xC0081A00, 
  /* Button 1 */ 0xC0081A01, 
  /* Button 2 */ 0xC0081A02, 
  /* Button 3 */ 0xC0081A03, 
  /* Button 4 */ 0xC0081A04, 
  /* Button 5 */ 0xC0081A05, 
  /* Button 6 */ 0xC0081A06, 
  /* Button 7 */ 0xC0081A07, 
  /* Button 8 */ 0xC0081A08, 
  /* Button 9 */ 0xC0081A09, 
};

int channelArray [3];
char searchArray[100];
char direction[2];
char unit[2];
int channel, currentButton, lastButton, ffspeed, dir, speed, duration, searchLength;

IRsend irsend(4);
 
void setup()
{
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  irsend.begin();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");  
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  
  if (mdns.begin("esp8266", WiFi.localIP())) 
  {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", handleRoot);
  server.on("/general", handleGeneral);
  server.on("/channel", handleChannel);
  server.on("/ffrw", handleffrw);
  server.on("/search", handleSearch);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

/*
 * Function to quickly send Sky IR Codes
 */

void skyButton(int button, int repeat = 1, int dlay = 500)
{
  int i = 1;
  while (i <= repeat)
  {
    irsend.sendRC6(button, 32);
    delay(dlay);
    i++;
  } 
}

/* 
 * handleGeneral 
 * Best suited to short button combinations 
 * Use URL: YOURADDRESS/general?general=YOURARGUMENT
 * Check the Serial Monitor to see how the argument is printed as spaces and capitalisation aren't always obvious
 */

void handleGeneral()
{
  Serial.println(server.arg("general")); // Use this to see how the argument is printed. 
  
  if (server.arg("general") == " TV Guide")
  {
    skyButton(Home);
    skyButton(Down);
    skyButton(Right);
    skyButton(Right);
  }

  if (server.arg("general") == " last viewed")
  {
    skyButton(Sky);
    skyButton(Down);
    skyButton(Select, 2);
  }

  if(server.arg("general") == " pause" || server.arg("general") == " unpause" || server.arg("general") == " on pause" || server.arg("general") == " and pause" || server.arg("general") == " play")
  {
    skyButton(PlayPause);
  }

  if (server.arg("general") == " home")
  {
    skyButton(Home);
  }
  
  if (server.arg("general") == " live TV")
  {
    skyButton(Back, 7, 100);
  }

  if (server.arg("general") == " what am I watching")
  {
    skyButton(Information);
    delay(6000);
    skyButton(Back);
  }

  if (server.arg("general") == " next episode")
  {
    skyButton(Select);
    skyButton(Right);
    skyButton(Select);
  }

  if (server.arg("general") == " enter" || server.arg("general") == " select")
  {
    skyButton(Select);
  }

  if (server.arg("general") == " page down")
  {
    skyButton(PageDOWN);
  }

  if (server.arg("general") == " page up")
  {
    skyButton(PageUP);
  }

  if (server.arg("general") == " right")
  {
    skyButton(Right);
  }

  if (server.arg("general") == " down")
  {
    skyButton(Down);
  }

  if (server.arg("general") == " left")
  {
    skyButton(Left);
  }

  if (server.arg("general") == " up")
  {
    skyButton(Up);
  }
}

/*
 * handleChannel
 * Takes a 3 digit number, parses into array elements, sends IR Code
 * Use URL MYADDRESS/channel?channel=YOURCHANNELNUMBER
 * This can be used effectively by passing a channel number manually eg. MYADDRESS/channel?channel=128 
 */

void handleChannel()
{
  channel = server.arg("channel").toInt();
  
  if (channel > 100 && channel < 1000)
  {
    for (int i = 2; i >= 0; i--) 
    { 
      channelArray[i] = channel % 10; 
      channel /= 10; 
      Serial.println(channelArray[i]);
    }
    
    for (int i = 0; i < 3; i++)
    {
      skyButton(Button[channelArray[i]]);
    }
    
     skyButton(Select); 
     skyButton(Back);
   }
   memset(channelArray, 0, sizeof(channelArray));
}


/* 
 * handleffrw
 * Fast Forward and Rewind Function
 * Uses x30 speed for Minutes and x6 for Seconds
 * URL = /ffrw?direction=(f or r)&unit=(m or s)&duration=x
 */
 
void handleffrw()
{
  server.arg("direction").toCharArray(direction, 2);
  server.arg("unit").toCharArray(unit, 2);
  duration = server.arg("duration").toInt();

  if(direction[0] == 'f')
  {
    dir = Forward;
  }
  else if (direction[0] == 'r')
  {
    dir = Rewind;
  }
  
  if(unit[0] == 'm')
  {
    speed = 7;
    duration = duration*FFRWMINUTE;
  }
  else if (unit[0] == 's')
  {
    speed = 2;
    duration = duration*FFRWSECOND;
  }

  skyButton(dir, speed, 100);
  delay(duration);
  skyButton(PlayPause);
}


/*
 * handleSearch
 * This started out as an experimental feature but ended up working well with Google Assistant
 * Uses t9 dictionary (like an old phone)
 * Uses letter function
 * URL = YOURADDRESS/search?search=YOURSEARCH
 */

void handleSearch()
{
 searchLength = server.arg("search").length()-1;
 server.arg("search").toCharArray(searchArray, 30);
  
 skyButton(Search, 3, 100);
 
 for (int i = 1; i <= searchLength; i++)
 { 
   if (searchArray[i] >= 65 && searchArray[i] <= 90)
   {
      searchArray[i] = tolower(searchArray[i]);
   }
   Serial.println(searchArray[i]);
   switch(searchArray[i])
    {
      case 'a':
        letter(Button[2], 1);
        break;
        
      case 'b':
        letter(Button[2], 2);
        break;
        
      case 'c':
        letter(Button[2], 3);
        break;
        
      case 'd':
        letter(Button[3], 1);  
        break;
        
      case 'e':
        letter(Button[3], 2);
        break;
        
      case 'f':
        letter(Button[3], 3);
        break;
        
      case 'g':
        letter(Button[4], 1);
        break;
        
      case 'h':
        letter(Button[4], 2);
        break;
        
      case 'i':
        letter(Button[4], 3);
        break;
        
      case 'j':
        letter(Button[5], 1);
        break;
        
      case 'k':
        letter(Button[5], 2);
        break;
        
      case 'l':
        letter(Button[5], 3);
        break;
        
      case 'm':
        letter(Button[6], 1);
        break;
        
      case 'n':
        letter(Button[6], 2);
        break;
        
      case 'o':
        letter(Button[6], 3);
        break;
        
      case 'p':
        letter(Button[7], 1);
        break;
        
      case 'q':
        letter(Button[7], 2);
        break;
        
      case 'r':
        letter(Button[7], 3);
        break;
        
      case 's':
        letter(Button[7], 4);
        break;
        
      case 't':
        letter(Button[8], 1);
        break;
        
      case 'u':
        letter(Button[8], 2);
        break;
        
      case 'v':
        letter(Button[8], 3);
        break;
        
      case 'w':
        letter(Button[9], 1);
        break;
        
      case 'x':
        letter(Button[9], 2);
        break;
        
      case 'y':
        letter(Button[9], 3);
        break;
        
      case 'z':
        letter(Button[9], 4);
        break;
        
      case ' ':
        letter(Button[0], 1); 
        break;

      case '0':
        letter(Button[0], 2); 
        break;

      case '1':
        letter(Button[1], 1); 
        break;

      case '2':
        letter(Button[2], 4); 
        break;

      case '3':
        letter(Button[3], 4); 
        break;

      case '4':
        letter(Button[4], 4); 
        break;
        
      case '5':
        letter(Button[5], 4); 
        break;

      case '6':
        letter(Button[6], 5); 
        break;

      case '7':
        letter(Button[7], 5); 
        break;

      case '8':
        letter(Button[8], 4); 
        break;

      case '9':
        letter(Button[9], 4); 
        break;
        
      default:
        // Do Nothing
        break;
  }
  
 }
 memset(searchArray, 0, sizeof(searchArray));
 
}


/*
 * letter 
 * Could do with a better name
 */

void letter(int currentButton, int repeat) 
{
  checkSame(currentButton);
  skyButton(currentButton, repeat, SEARCHDELAY);
  lastButton = currentButton;
}


/*
 * checkSame
 * if inputting the previous letter requires use of the same "button" then add a delay
 */
 
void checkSame(int currentButton)
{
    if (lastButton == currentButton)
    {
        delay(SAMEBUTTONDELAY);
    }   
}


void handleRoot() 
{
  server.send(200, "text/plain", "hello from esp8266!");
}


void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


void loop()
{
  server.handleClient();
}
