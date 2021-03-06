#include <SPI.h>
#include <UIPEthernet.h>
#include <LinkedList.h>
#include <SerialDataParser.h>
#include <Adafruit_NeoPixel.h>

#define PORT 1000

#define NUM_PIXELS 60
#define FADE_STEPS 50
#define FADE_DELAY 20
#define SWIRL_DELAY 50

SerialDataParser sdp('^', '$', ',');
EthernetServer server = EthernetServer(PORT);
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
  sdp.addParser("swirl", swirlPixelsParser);
  sdp.addParser("shift", shiftPixelsParser);
  Serial.println(F("Starting up network"));
  Ethernet.begin(mac);
  server.begin();
  
  Serial.println(Ethernet.localIP());
  
  pulsePixels(0, 64,0);
}

void loop()
{
  size_t size;

  if (EthernetClient client = server.available())
    {
      while(client.available() > 0)
      {
        char c = client.read();
        Serial.print(c);
        sdp.appendChar(c);
      }
      Serial.println();
    }
}

void singlePixelParser(char **values, int valueCount)
{
  if(valueCount != 5)
  {
    return;
  }
  
  pixels[atoi(values[1])] = neopixel.Color(atoi(values[2]), atoi(values[3]), atoi(values[4]));

  updatePixels();
}

void clearPixelsParser(char **values, int valueCount)
{
  clearPixels();  
  updatePixels();
}

void clearPixels()
{
  for(int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i] = 0;
  }
}

void allPixelsParser(char **values, int valueCount)
{
  if(valueCount != 4)
  {
    return;
  }
  
  uint32_t color = neopixel.Color(atoi(values[1]), atoi(values[2]), atoi(values[3]));
  for(int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i] = color;
  }
  
  updatePixels();
}

void pulsePixelsParser(char **values, int valueCount)
{
  uint8_t red = atoi(values[1]);
  uint8_t green = atoi(values[2]);
  uint8_t blue = atoi(values[3]);
  
  pulsePixels(red, green, blue);
}

void pulsePixels(uint8_t red, uint8_t green, uint8_t blue)
{
  for(int i = 0; i < FADE_STEPS; i++)
  {
    uint32_t color = neopixel.Color(map(i, 0, FADE_STEPS, 0, red),
                                    map(i, 0, FADE_STEPS, 0, green),
                                    map(i, 0, FADE_STEPS, 0, blue));

    for(int p = 0; p < NUM_PIXELS; p++)
    {
      neopixel.setPixelColor(p, color);
    }
    neopixel.show();
    delay(FADE_DELAY);
  }

  for(int i = FADE_STEPS; i > 0; i--)
  {
    uint8_t fadeRed = map(i, 0, FADE_STEPS, 0, red);
    uint8_t fadeGreen = map(i, 0, FADE_STEPS, 0, green);
    uint8_t fadeBlue = map(i, 0, FADE_STEPS, 0, blue);
    
    uint32_t color = neopixel.Color(fadeRed, fadeGreen, fadeBlue);
    for(int p = 0; p < NUM_PIXELS; p++)
    {
      neopixel.setPixelColor(p, color);
    }
    neopixel.show();
    delay(FADE_DELAY);
  }
  
  updatePixels();
}

void flashPixelsParser(char **values, int valueCount)
{
  if(valueCount != 5)
  {
    return;
  }
  
  uint32_t color = neopixel.Color(atoi(values[2]), atoi(values[3]), atoi(values[4]));
  
  for(int t = 0; t < atoi(values[1]); t++)
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

void swirlPixelsParser(char **values, int valueCount)
{
  if(valueCount != 5)
    return;

  uint32_t color = neopixel.Color(atoi(values[2]), atoi(values[3]), atoi(values[4]));

  for(int i = 0; i < NUM_PIXELS; i++)
  {
    neopixel.setPixelColor(i, 0);
  }
  
  for(int i = 0; i < atoi(values[1]); i++)
  {
    for(int p = 0; p < NUM_PIXELS; p++)
    {
      int p2 = (p + (NUM_PIXELS/2)) % NUM_PIXELS;
      
      for(int j = 0; j < NUM_PIXELS; j++)
      {
        neopixel.setPixelColor(j, 0);
      }
      
      neopixel.setPixelColor(p, color);
      neopixel.setPixelColor(p2 % NUM_PIXELS, color);

      neopixel.show();
      delay(SWIRL_DELAY);
    }
  }
  updatePixels();
}

void shiftPixelsParser(char **values, int valueCount)
{
  uint32_t c = pixels[0];
  
  for(int i = 0; i < NUM_PIXELS-1; i++)
  {
    pixels[i] = pixels[i+1];
  }
  pixels[NUM_PIXELS-1] = c;
  
  updatePixels();
}

void updatePixels()
{
  for(int i = 0; i < NUM_PIXELS; i++)
  {
    neopixel.setPixelColor(i, pixels[i]);
  }
  neopixel.show();
}
