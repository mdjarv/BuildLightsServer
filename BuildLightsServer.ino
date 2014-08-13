#include <SPI.h>
#include <UIPEthernet.h>
#include <Adafruit_NeoPixel.h>

#define NUM_PIXELS 16

Adafruit_NeoPixel ring = Adafruit_NeoPixel(NUM_PIXELS, 4, NEO_GRB + NEO_KHZ800);

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

EthernetServer server(80);

String request;
bool doAppend = false;
int values[4];

uint32_t pixels[NUM_PIXELS];

void clearPixels()
{
  for(int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i] = 0;
  }
  
  updatePixels();
}

void flashColor(uint32_t color)
{
  for(int i = 0; i < NUM_PIXELS; i++)
  {
    ring.setPixelColor(i, color);
  }

  ring.show();

  delay(200);

  for(int i = 0; i < NUM_PIXELS; i++)
  {
    ring.setPixelColor(i, pixels[i]);
  }

  ring.show();
  
}

void setup() {
  Serial.begin(9600);
  
  ring.begin();
  ring.show();

  Ethernet.begin(mac);
  server.begin();
  Serial.print("Server has IP ");
  Serial.println(Ethernet.localIP());
  
  flashColor(ring.Color(0, 255, 0));
  clearPixels();
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if(c == '?')
          doAppend = true;
        else if(c == ' ')
          doAppend = false;
        else if(doAppend)
          request += c;
          
        if (c == '\n' && currentLineIsBlank) {
          processRequest();
          char hexCol[10];
          sprintf(hexCol, "%02x%02x%02x", values[1], values[2], values[3]);

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<body>");
          client.print("LED ");
          client.print(values[0]);
          client.print(" set to color <span style=\"color: #");
          client.print(hexCol);
          client.print("\">#");
          client.print(hexCol);
          client.println("</span>");


          client.println("</body>");
          client.println("</html>");
          
          request = "";
          
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);

    client.stop();
    Serial.println("Client disconnected");
  }
}

void processRequest()
{
  Serial.print("request: ");
  Serial.println(request);
  
  int valueIndex = 0;

  if(request.indexOf(',') < 0)
  {  
    if(request == "clear")
    {
      clearPixels();
    }
    else if(request == "flashGreen")
    {
      flashColor(ring.Color(0, 255, 0));
    }
    else if(request == "flashRed")
    {
      flashColor(ring.Color(255, 0, 0));
    }
    return;
  }

  int i = 0;
  int delim = request.indexOf(',', i);
  
  while(true)
  {
    if(delim > 0)
    {
      values[valueIndex++] = request.substring(i, delim).toInt();
      
      i = delim+1;
      delim = request.indexOf(',', i);
    }
    else
    {
      values[valueIndex++] = request.substring(i).toInt();
      break;
    }
  
    if(valueIndex == 4)
      break;
  }
  
  pixels[values[0]] = ring.Color(values[1], values[2], values[3]);
  
  updatePixels();
}

void updatePixels()
{
  for(int i = 0; i < NUM_PIXELS; i++)
  {
    ring.setPixelColor(i, pixels[i]);
  }
  ring.show();
}
