/*#define R1 25
#define G1 26
#define BL1 27
#define R2 14
#define G2 12
#define BL2 13
#define CH_A 23
#define CH_B 19
#define CH_C 5
#define CH_D 17
#define CH_E -1 // assign to any available pin if using two panels or 64x64 panels with 1/32 scan
#define CLK 16
#define LAT 4
#define OE 15*/

/*--------------------- MATRIX GPIO CONFIG  -------------------------*/
/*--------------------- HUB75 MATRIX GPIO CONFIG  ---------------------*/
#define R1  2
#define G1  23
#define BL1  4
#define R2  16
#define G2  27
#define BL2  17

#define CH_A   5
#define CH_B   18
#define CH_C   19
#define CH_D   21
#define CH_E   33
#define LAT 26
#define OE  25

#define CLK 22

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FastLED.h>

// Configure for your panel(s) as appropriate!
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64 // Panel height of 64 will required PIN_E to be defined.
#define PANELS_NUMBER 1 // Number of chained panels, if just a single panel, obviously set to 1
//#define PIN_E 18

#define PANE_WIDTH PANEL_WIDTH * PANELS_NUMBER
#define PANE_HEIGHT PANEL_HEIGHT

// placeholder for the matrix object
MatrixPanel_I2S_DMA *dma_display = nullptr;

uint16_t time_counter = 0, cycles = 0, fps = 0;
unsigned long fps_timer;

CRGB currentColor;
CRGBPalette16 palettes[] = {HeatColors_p, LavaColors_p, RainbowColors_p, RainbowStripeColors_p, CloudColors_p};
CRGBPalette16 currentPalette = palettes[0];

CRGB ColorFromCurrentPalette(uint8_t index = 0, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND) {
return ColorFromPalette(currentPalette, index, brightness, blendType);
}

void setup() {

Serial.begin(115200);

Serial.println(F(""));
Serial.println(F("* ESP32-HUB75-MatrixPanel-I2S-DMA DEMO *"));
Serial.println(F(""));
/*
HUB75_I2S_CFG mxconfig;
mxconfig.mx_height = PANEL_HEIGHT; // we have 64 pix heigh panels
mxconfig.chain_length = PANELS_NUMBER; // we have 2 panels chained
mxconfig.gpio.e = CH_E; // we MUST assign pin e to some free pin on a board to drive 64 pix height panels with 1/32 scan
//mxconfig.driver = HUB75_I2S_CFG::FM6126A; // in case that we use panels based on FM6126A chip, we can change that
*/

//MatrixPanel_I2S_DMA *dma_display = nullptr;

// Module configuration
HUB75_I2S_CFG::i2s_pins _pins={R1, G1, BL1, R2, G2, BL2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
HUB75_I2S_CFG mxconfig(
                PANEL_WIDTH,    // module width
                PANEL_HEIGHT,    // module height
                PANELS_NUMBER,    // chain length
                _pins           // pin mapping      
);

// OK, now we can create our matrix object
//dma_display = new MatrixPanel_I2S_DMA(mxconfig);

/************** DISPLAY **************/
  Serial.println("...Starting Display");
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90); //0-255

// let's adjust default brightness to about 75%
dma_display->setBrightness8(255); // range is 0-255, 0 - 0%, 255 - 100%

// Allocate memory and start DMA display
if( not dma_display->begin() )
Serial.println("****** !KABOOM! I2S memory allocation failed ***********");

// well, hope we are OK, let's draw some colors first :)
Serial.println("Fill screen: RED");
dma_display->fillScreenRGB888(255, 0, 0);
delay(1000);

Serial.println("Fill screen: GREEN");
dma_display->fillScreenRGB888(0, 255, 0);
delay(1000);

Serial.println("Fill screen: BLUE");
dma_display->fillScreenRGB888(0, 0, 255);
delay(1000);

Serial.println("Fill screen: Neutral White");
dma_display->fillScreenRGB888(64, 64, 64);
delay(1000);

Serial.println("Fill screen: black");
dma_display->fillScreenRGB888(0, 0, 0);
delay(1000);

// Set current FastLED palette
currentPalette = RainbowColors_p;
Serial.println("Starting plasma effect...");
fps_timer = millis();
}

void loop() {

for (int x = 0; x < PANE_WIDTH; x++) {
        for (int y = 0; y <  PANE_HEIGHT; y++) {
            int16_t v = 128;
            uint8_t wibble = sin8(time_counter);
            v += sin16(x * wibble * 3 + time_counter);
            v += cos16(y * (128 - wibble)  + time_counter);
            v += sin16(y * x * cos8(-time_counter) / 8);

            currentColor = ColorFromPalette(currentPalette, (v >> 8)); //, brightness, currentBlendType);
            dma_display->drawPixelRGB888(x, y, currentColor.r, currentColor.g, currentColor.b);
        }
}

++time_counter;
++cycles;
++fps;

if (cycles >= 1024) {
    time_counter = 0;
    cycles = 0;
    currentPalette = palettes[random(0,sizeof(palettes)/sizeof(palettes[0]))];
}

// print FPS rate every 5 seconds
// Note: this is NOT a matrix refresh rate, it's the number of data frames being drawn to the DMA buffer per second
if (fps_timer + 5000 < millis()){
  Serial.printf_P(PSTR("Effect fps: %d\n"), fps/5);
  fps_timer = millis();
  fps = 0;
}

} // end loop
