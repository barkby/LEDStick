/*
LEDStick by  Justin Barkby

Original code by Michael Ross (http://mrossphoto.com/wordpress32/)
Modified from LCD to TFT touchscreen including a couple of features by Justin Barkby

*/

#if defined(__AVR__)
#define imagedatatype  unsigned int
#elif defined(__PIC32MX__)
#define imagedatatype  unsigned short
#elif defined(__arm__)
#define imagedatatype  unsigned short
#endif

#include <UTFT.h>
#include <UTouch.h>
#include <UTFT_Buttons.h>
#include <Adafruit_NeoPixel.h>
#include <SD.h>
#include <SPI.h>

extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];
extern imagedatatype Left[];
extern imagedatatype Right[];

UTFT          myGLCD(ITDB32S,38,39,40,41);
UTouch        myTouch(6,5,4,3,2);
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

#define PIN 8 
#define SDssPin 53  //SD card CS pin

byte gamma(byte x);

#define STRIP_LENGTH 144
boolean dualstrip = false;//set to true for dual strips

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LENGTH, PIN, NEO_GRB + NEO_KHZ800);

File root;
File dataFile;
String m_CurrentFilename = "";
int m_FileIndex = 0;
int m_NumberOfFiles = 0;
int found;
int keypress;
int StickMode;
int frameDelay = 10;
String m_FileNames[200]; //yep this is bad, but unless you are going to have over 200 images on your lightwand..
int TTP;
int a, b, c, d, e;

long buffer[STRIP_LENGTH]; 

extern uint8_t BigFont[];

int x, y;
char stCurrent[20]="";
int stCurrentLen=0;
char stLast[20]="";

void setup()
{
  Serial.begin(115200);
  strip.begin();
  strip.show();
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  myButtons.setTextFont(SmallFont);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 0); 
  myGLCD.print("LED Stick", CENTER, 0);
  myGLCD.setFont(SmallFont);
  myGLCD.print("Ditigal Lightwand v0.1 ALPHA",CENTER, 16);
  myGLCD.print("by Justin Barkby",CENTER, 50);
  delay(500);
  initial_selection();
}

void initial_selection()
{
  int but1, but2, pressed_button;
  boolean default_colors = true;

  // but1 = myButtons.addButton( 10,  90, 300,  30, "Sequence Painting");
  but2 = myButtons.addButton( 10,  130, 300,  30, "Painting BMP");
  myButtons.drawButtons();
  while (true)
  {
    if (myTouch.dataAvailable() == true)
    {
      pressed_button = myButtons.checkButtons();

      /* if (pressed_button==but1)
      {
        myGLCD.clrScr();
        pattern();
      }
      */
      if (pressed_button==but2)
      {
        myGLCD.clrScr();
        BMP();
      }
    }
  }
}

void loop()

{
}

void pattern()
{
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 0); 
  myGLCD.setColor(255, 0, 255);
  myGLCD.print("Sequence test",CENTER,200);
  Pulse();
  delay(20000);

}

void Pulse() {

  for (int p = 0; p < 255; p++) {
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0, 0, p);
    }
    strip.show();
  }
  for (int p = 255; p > 0; p--) {
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0, 0, p);
    }
    strip.show();
  }
}
void BMP()
{
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 0); 
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("BMP Mode selected",CENTER,200);
  delay(2000);
  setupSDcard();
  bmploop();
}

void setupSDcard()
{
  pinMode(SDssPin, OUTPUT);



  while (!SD.begin(SDssPin)) {
    myGLCD.setFont(BigFont);
    myGLCD.print("No SD Card?",CENTER, 150);
    delay(1000);
    myGLCD.clrScr();
    delay(500);
  }
  myGLCD.setFont(BigFont);
  myGLCD.print("Card Inserted",CENTER, 0);
  delay(1000);
  root = SD.open("/");
  myGLCD.print("Scanning Files",CENTER, 20);
  GetFileNamesFromSD(root);
  String found = String (m_NumberOfFiles);  
  myGLCD.print(found + " Found",CENTER, 40);
  myGLCD.print("Sorting Files",CENTER, 60);
  isort(m_FileNames, m_NumberOfFiles);
  myGLCD.print("DONE",CENTER, 80);
  m_CurrentFilename = m_FileNames[0];
  myGLCD.print("Now the fun Begins",CENTER, 150);
  delay(500);
  myGLCD.clrScr();
  DisplayCurrentFilename();
}


void GetFileNamesFromSD(File dir)
{
  int fileCount = 0;
  String CurrentFilename = "";
  while(1)
  {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      m_NumberOfFiles = fileCount;
      break;
    }
    else
    {
      if (entry.isDirectory()) {
        //GetNextFileName(root);
      }
      else {
        CurrentFilename = entry.name();

        myGLCD.setFont(BigFont);
        myGLCD.setBackColor (0, 0, 0);
        myGLCD.print("   " + CurrentFilename + "   " ,CENTER, 200 );

        if (CurrentFilename.endsWith(".bmp") || CurrentFilename.endsWith(".BMP") )//find files with our extension only
        {
          m_FileNames[fileCount] = entry.name();
          fileCount++;  
        }
      }
    }
  }
}

void isort(String *filenames, int n)
{
  for (int i = 1; i < n; ++i)
  {
    String j = filenames[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < filenames[k]); k--)
    {
      filenames[k + 1] = filenames[k];
    }
    filenames[k + 1] = j;
  }
}

void DisplayCurrentFilename()
{
  m_CurrentFilename = m_FileNames[m_FileIndex];
  myGLCD.setFont(SmallFont);
  myGLCD.setBackColor (0, 0, 0);
  myGLCD.print("   " + m_CurrentFilename + "   " ,10, 36 );
  ReadingFile (m_CurrentFilename);
}

void bmploop()
{
  int prev, next, info, plus, minus, paint, pressed_button;
  boolean default_colors = true;
  myButtons.deleteAllButtons();
  prev = myButtons.addButton( 10,  0, 150,  30, "Previous");
  next = myButtons.addButton( 10,  60, 150,  30, "Next");
  plus = myButtons.addButton( 180,  0, 120,  30, "-5");
  minus = myButtons.addButton( 180,  60, 120,  30, "+5");
  // info = myButtons.addButton( 10,  160, 300,  30, "File Information");
  paint = myButtons.addButton( 10,  200, 300,  30, "Paint BMP");
  myButtons.drawButtons();
  ShowFrameDelay();

  while (true)
  {
    if (myTouch.dataAvailable() == true)
    {
      pressed_button = myButtons.checkButtons();

      if (pressed_button==prev)
      {
        if (m_FileIndex > 0)
        {
          m_FileIndex--;
        }
        else
        {
          m_FileIndex = m_NumberOfFiles -1; //wrap round to the last file
        }

        DisplayCurrentFilename();
        delay(500);
        keypress = 0;
      }
      if (pressed_button==next)
      {
        if (m_FileIndex < m_NumberOfFiles -1)
        {
          m_FileIndex++;
        }
        else
        {
          m_FileIndex = 0;
        }
        DisplayCurrentFilename();
        delay(500);
        keypress = 0;
      }
      if (pressed_button==plus)
      {
        if (frameDelay > 5)
        {
          frameDelay-=5;
        }
        ShowFrameDelay();
      }
      if (pressed_button==minus)
      {
        if (frameDelay < 200)
        {
          frameDelay+=5;
        }
        ShowFrameDelay();
      }
      if (pressed_button==paint)
      {
        Paint();
      }
    }
  }
}

void ReadingFile(String Filename)
{
  char temp[14];
  Filename.toCharArray(temp,14);

  dataFile = SD.open(temp);
  ReadTheFile();
  dataFile.close();
}

void ShowFrameDelay()
{
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(255, 255, 255);
  String adelay = String (frameDelay); 
  myGLCD.print("  " + adelay + "  ", 220, 36);
  delay(500);
  DisplayCurrentFilename();
}

uint32_t readLong()
{
  uint32_t retValue;
  byte incomingbyte;

  incomingbyte=readByte();
  retValue=(uint32_t)((byte)incomingbyte);

  incomingbyte=readByte();
  retValue+=(uint32_t)((byte)incomingbyte)<<8;

  incomingbyte=readByte();
  retValue+=(uint32_t)((byte)incomingbyte)<<16;

  incomingbyte=readByte();
  retValue+=(uint32_t)((byte)incomingbyte)<<24;

  return retValue;
}

uint16_t readInt()
{
  byte incomingbyte;
  uint16_t retValue;

  incomingbyte=readByte();
  retValue+=(uint16_t)((byte)incomingbyte);

  incomingbyte=readByte();
  retValue+=(uint16_t)((byte)incomingbyte)<<8;

  return retValue;
}
int readByte()
{
  int retbyte=-1;
  while(retbyte<0) retbyte= dataFile.read();
  return retbyte;
}

void ReadTheFile()
{

#define MYBMP_BF_TYPE           0x4D42
#define MYBMP_BF_OFF_BITS       54
#define MYBMP_BI_SIZE           40
#define MYBMP_BI_RGB            0L
#define MYBMP_BI_RLE8           1L
#define MYBMP_BI_RLE4           2L
#define MYBMP_BI_BITFIELDS      3L

  uint16_t bmpType = readInt();
  uint32_t bmpSize = readLong();
  uint16_t bmpReserved1 = readInt();
  uint16_t bmpReserved2 = readInt();
  uint32_t bmpOffBits = readLong();
  bmpOffBits = 54;
  uint32_t imgSize = readLong();
  uint32_t imgWidth = readLong();
  uint32_t imgHeight = readLong();
  uint16_t imgPlanes = readInt();
  uint16_t imgBitCount = readInt();
  uint32_t imgCompression = readLong();
  uint32_t imgSizeImage = readLong();
  uint32_t imgXPelsPerMeter = readLong();
  uint32_t imgYPelsPerMeter = readLong();
  uint32_t imgClrUsed = readLong();
  uint32_t imgClrImportant = readLong();

  String height = String(imgHeight);
  myGLCD.setFont(BigFont);
  String w = String(imgWidth);
  c = frameDelay/2;
  d = frameDelay + c;
  a = imgHeight * d;
  
  if (w > height)
    b = imgWidth;
  else
    b = imgHeight;
  
  String d = String(frameDelay);
  e = ((frameDelay*b+(25*b)))/1000;
  String p = String(e);
  myGLCD.print("  " + w + " X " + height + "  ",CENTER,100);
    String ti = String(b);
  myGLCD.print(" " + p + " Seconds ", CENTER,115);

}

void latchanddelay(int dur)
{
  strip.show();
  delay(dur);
}

void ClearStrip(int duration)
{
  int x;
  for(x=0;x<STRIP_LENGTH;x++)
  {
    strip.setPixelColor(x, 0);
  }
  strip.show();
  latchanddelay(duration);

}

void Paint()
{
  myGLCD.clrScr();
  delay(500);
  while (true)
  {
    if (myTouch.dataAvailable())
    {
      SendFile(m_CurrentFilename);
      bmploop();
      // bmploopreset();
    }
  }
}
void SendFile(String Filename)
{
  // lcd.clear();
  // lcd.print("Sending File");
  // lcd.setCursor(0, 1);
  // lcd.print(Filename);
  char temp[14];
  Filename.toCharArray(temp,14);

  dataFile = SD.open(temp);

  if (dataFile)
  {
    Painting();
    dataFile.close();
    ClearStrip(100);
  }  
  else
  {
    delay(1000);
    setupSDcard();
    return;
  }
  DisplayCurrentFilename();
} 
void Painting()
{
#define MYBMP_BF_TYPE           0x4D42
#define MYBMP_BF_OFF_BITS       54
#define MYBMP_BI_SIZE           40
#define MYBMP_BI_RGB            0L
#define MYBMP_BI_RLE8           1L
#define MYBMP_BI_RLE4           2L
#define MYBMP_BI_BITFIELDS      3L



  uint16_t bmpType = readInt();
  uint32_t bmpSize = readLong();
  uint16_t bmpReserved1 = readInt();
  uint16_t bmpReserved2 = readInt();
  uint32_t bmpOffBits = readLong();
  bmpOffBits = 54;


  /* Check file header */
  if (bmpType != MYBMP_BF_TYPE || bmpOffBits != MYBMP_BF_OFF_BITS)
  {
    myGLCD.clrScr();
    delay(500);
    myGLCD.setFont(BigFont);
    myGLCD.print("Not a BMP",CENTER, 0);
    delay(1000);
    return;
  }

  /* Read info header */
  uint32_t imgSize = readLong();
  uint32_t imgWidth = readLong();
  uint32_t imgHeight = readLong();
  uint16_t imgPlanes = readInt();
  uint16_t imgBitCount = readInt();
  uint32_t imgCompression = readLong();
  uint32_t imgSizeImage = readLong();
  uint32_t imgXPelsPerMeter = readLong();
  uint32_t imgYPelsPerMeter = readLong();
  uint32_t imgClrUsed = readLong();
  uint32_t imgClrImportant = readLong();

  if( imgSize != MYBMP_BI_SIZE || imgWidth <= 0 ||
    imgHeight <= 0 || imgPlanes != 1 ||
    imgBitCount != 24 || imgCompression != MYBMP_BI_RGB ||
    imgSizeImage == 0 )
  {
    myGLCD.clrScr();
    delay(500);
    myGLCD.setFont(BigFont);
    myGLCD.print("Unsupported BMP",CENTER, 0);
    myGLCD.print("Use 24bpp",CENTER, 0);
    delay(1000);
    return;
  }

  int displayWidth = imgWidth;
  if (imgWidth > STRIP_LENGTH)
  {
    displayWidth = STRIP_LENGTH;//only display the number of led's we have
  }


  /* compute the line length */
  uint32_t lineLength = imgWidth * 3;
  if ((lineLength % 4) != 0)
    lineLength = (lineLength / 4 + 1) * 4;

  int x = 0;
  for(int y=imgHeight;y>0 ;y--) {

    int bufpos=0;    


    if ( dualstrip == true)
    {
      int pos = 0;
      for(int x=0;x <((displayWidth)/2) ;x ++) {

        uint32_t offset = (MYBMP_BF_OFF_BITS + (((y-1)* lineLength) + (pos*3))) ;

        dataFile.seek(offset);

        int g=gamma(readByte());
        int b=gamma(readByte());
        int r=gamma(readByte());
        strip.setPixelColor(x,r,g,b);

        g=gamma(readByte());
        b=gamma(readByte());
        r=gamma(readByte());
        strip.setPixelColor((STRIP_LENGTH-x)-1,r,g,b);
        pos+=2;

      }

      latchanddelay(frameDelay/2);


    }
    else
    {
      for(int x=0;x <displayWidth  ;x++) {

        uint32_t offset = (MYBMP_BF_OFF_BITS + (((y-1)* lineLength) + (x*3))) ;

        dataFile.seek(offset);

        int b=gamma(readByte());
        int r=gamma(readByte());
        int g=gamma(readByte());
        strip.setPixelColor(x,g,r,b);


      }
      latchanddelay(frameDelay);
    }


  }
  ClearStrip(100);
}


// Gamma correction compensates for our eyes' nonlinear perception of
// intensity.  It's the LAST step before a pixel value is stored, and
// allows intermediate rendering/processing to occur in linear space.
// The table contains 256 elements (8 bit input), though the outputs are
// only 7 bits (0 to 127).  This is normal and intentional by design: it
// allows all the rendering code to operate in the more familiar unsigned
// 8-bit colorspace (used in a lot of existing graphics code), and better
// preserves accuracy where repeated color blending operations occur.
// Only the final end product is converted to 7 bits, the native format
// for the LPD8806 LED driver.  Gamma correction and 7-bit decimation
// thus occur in a single operation.
PROGMEM prog_uchar gammaTable[]  = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,
  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11,
  11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16,
  16, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22,
  23, 23, 24, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30,
  30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 37, 37, 38, 38, 39,
  40, 40, 41, 41, 42, 43, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50,
  50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 58, 59, 60, 61, 62,
  62, 63, 64, 65, 66, 67, 67, 68, 69, 70, 71, 72, 73, 74, 74, 75,
  76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
  92, 93, 94, 95, 96, 97, 98, 99,100,101,102,104,105,106,107,108,
  109,110,111,113,114,115,116,117,118,120,121,122,123,125,126,127
};


inline byte gamma(byte x) {
  return pgm_read_byte(&gammaTable[x]);
}

void FileInformation(String Filename)
{

}
