#include <SPI.h>
#include <UIPEthernet.h>
#include <SerialDataParser.h>
#include <Adafruit_NeoPixel.h>

#define NUM_PIXELS 16

SerialDataParser sdp('^', '$', ',');
EthernetServer server = EthernetServer(1000);
Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(NUM_PIXELS, 4, NEO_GRB + NEO_KHZ800);

uint32_t pixels[NUM_PIXELS];

void setup()
{
  Serial.begin(9600);
  
  neopixel.begin();
  neopixel.show();

  uint8_t mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

  sdp.addParser("pixel", singlePixelParser);
  sdp.addParser("all", allPixelsParser);
  sdp.addParser("clear", clearPixelsParser);
  sdp.addParser("pulse", pulsePixelsParser);
  sdp.addParser("flash", flashPixelsParser);
  
  Ethernet.begin(mac);
  server.begin();
  
  pulsePixels(0, 128,0);
}

void loop()
{
  size_t size;

  if (EthernetClient client = server.available())
    {
      while((size = client.available()) > 0)
        {
          uint8_t* msg = (uint8_t*)malloc(size);
          size = client.read(msg,size);
          for(int i = 0; i < size; i++)
          {
            sdp.appendChar(msg[i]);
          }
          free(msg);
        }
      client.println("OK");
      client.stop();
    }
}

void singlePixelParser(String *values, int valueCount)
{
  if(valueCount != 5)
  {
    return;
  }
  
  pixels[values[1].toInt()] = neopixel.Color(values[2].toInt(), values[3].toInt(), values[4].toInt());

  updatePixels();
}

void clearPixelsParser(String *values, int valueCount)
{
  for(int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i] = 0;
  }
  
  updatePixels();
}

void allPixelsParser(String *values, int valueCount)
{
  if(valueCount != 4)
  {
    return;
  }
  
  uint32_t color = neopixel.Color(values[1].toInt(), values[2].toInt(), values[3].toInt());
  for(int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i] = color;
  }
  
  updatePixels();
}

void pulsePixelsParser(String *values, int valueCount)
{
  uint8_t red = values[1].toInt();
  uint8_t green = values[2].toInt();
  uint8_t blue = values[3].toInt();
  
  pulsePixels(red, green, blue);
}

void pulsePixels(uint8_t red, uint8_t green, uint8_t blue)
{
  int steps = 50;
  int fadeDelay = 20;
  
  for(int i = 0; i < steps; i++)
  {
    uint8_t fadeRed = map(i, 0, steps, 0, red);
    uint8_t fadeGreen = map(i, 0, steps, 0, green);
    uint8_t fadeBlue = map(i, 0, steps, 0, blue);
    
    uint32_t color = neopixel.Color(fadeRed, fadeGreen, fadeBlue);

    for(int p = 0; p < NUM_PIXELS; p++)
    {
      neopixel.setPixelColor(p, color);
    }
    neopixel.show();
    delay(fadeDelay);
  }

  for(int i = steps; i > 0; i--)
  {
    uint8_t fadeRed = map(i, 0, steps, 0, red);
    uint8_t fadeGreen = map(i, 0, steps, 0, green);
    uint8_t fadeBlue = map(i, 0, steps, 0, blue);
    
    uint32_t color = neopixel.Color(fadeRed, fadeGreen, fadeBlue);
    for(int p = 0; p < NUM_PIXELS; p++)
    {
      neopixel.setPixelColor(p, color);
    }
    neopixel.show();
    delay(fadeDelay);
  }
  
  updatePixels();
}

void flashPixelsParser(String *values, int valueCount)
{
  if(valueCount != 5)
  {
    return;
  }
  
  uint32_t color = neopixel.Color(values[2].toInt(), values[3].toInt(), values[4].toInt());
  
  for(int t = 0; t < values[1].toInt(); t++)
  {
    for(int i = 0; i < NUM_PIXELS; i++)
    {
      neopixel.setPixelColor(i, color);
    }
  
    neopixel.show();
  
    delay(200);
    updatePixels();
    delay(200);
  }
}

void updatePixels()
{
  for(int i = 0; i < NUM_PIXELS; i++)
  {
    neopixel.setPixelColor(i, pixels[i]);
  }
  neopixel.show();
}
