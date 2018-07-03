unsigned int Acc;
signed char accTopTemp=0;
signed char Mask=0;
float test;
byte arpMode=0;
byte sinVal;
byte noteIndex=0;
byte chordIndex=0;
long checktime=0;
int octRange=3;
int tempo=100;
unsigned note=0;
unsigned temp=0;
byte oct=3;
float outFreq=0;
byte incomingByte;
byte waveforms = 1; //1,2,3,4 sq,saw,tria,sin
//use timer T2 do create a PWM sampled signal that will be filtered out to become an analog signal
const uint16_t scale[] = {857,908,962,1020,1080,1144,1212,1285,1361,1442,1528,1618,1715,1816,1924,2040,2160,2288,2424,2570,2722,2884,3056,3236,3430,3632};
const float oct_multi[] = {0.125,0.25,0.5,1,2,4};
const int majorChords[] = {12,7,12,0,4}; //semitone sequence 0+4+7
static const uint8_t sin_table[] PROGMEM = 
{127,130,133,136,140,143,146,149,152,155,158,161,164,167,170,173,
176,179,182,185,188,190,193,196,198,201,204,206,208,211,213,216,
218,220,222,224,226,228,230,232,234,235,237,239,240,242,243,244,
245,247,248,249,250,251,251,252,253,253,254,254,254,255,255,255,
255,255,255,255,254,254,253,253,252,252,251,250,249,248,247,246,
245,244,242,241,239,238,236,235,233,231,229,227,225,223,221,219,
217,214,212,210,207,205,202,200,197,194,192,189,186,183,180,178,
175,172,169,166,163,160,157,154,151,147,144,141,138,135,132,129,
125,127,127,127,127,127,127,127};

void setup() { 
Serial.begin(115200);
pinMode(LED_BUILTIN,OUTPUT);

DDRD = DDRD | 1<<DDD3; // PD3 (Arduino D3) as output (OC2B timer output)
DDRB = DDRB | 1<<DDB5; //pin B5 (pin13) output

//use one timer T1 to select a sampling frequency @20Khz and trigger an interrupt
TCCR1A = 0<<COM1A0 | 0<<COM1B0 | 0<<WGM10 | 0<<WGM11; // CTC TOP=OCR1A
OCR1A = 99;
TCCR1B = 1<<WGM12 | 2<<CS10;  //010 prescaler 1:8 (16Mhz/8)/99+1=20Khz (101 5 prescaler1024)
TIMSK1 =(1<<OCIE1A);

TCCR2A = 0<<COM2A0 | 2<<COM2B0 | 3<<WGM20; // fastPWM on OC2B (pin3) 
OCR2A = 200;
OCR2B = 100;
TCCR2B = 1<<WGM22 | 1<<CS20;  //prescaler 1:1
}

ISR(TIMER1_COMPA_vect) {
  temp=note*oct_multi[oct];
  Acc=Acc+temp;
  accTopTemp=Acc>>8;
  Mask=accTopTemp>>7;
  switch (waveforms) {
    case (1):
      OCR2B=(Acc>>8)&0x80;        //soft square top@0x80
    break;
    case (2):
      OCR2B=Acc>>8;               //sawtooth top@FF
    break;
    case (3):
      OCR2B=accTopTemp^Mask;      //soft triangle wave top@0x80
    break;
    case (4):                     //sin wave (table) top@0xFF
      sinVal=pgm_read_byte(&sin_table[accTopTemp^Mask]);
      //test=sin(Acc>8); //uncomment this to understand the weight of floating point math @20Khz
      OCR2B=sinVal^Mask;  
    break;
    case (5):                     //|sin| (double frequency) top@0xFF
      sinVal=pgm_read_byte(&sin_table[accTopTemp^Mask]);
      OCR2B=sinVal;  
    break;
    case (6):                     //square top@FF
      OCR2B=Mask;         
    break;  
    }
    
  //Serial.println("X     "+String(Acc>>8));
  //Serial.println("OCR2B "+String(OCR2B)); //test only this line with serial plotter to check waveform
  //Serial.println(accTopTemp,BIN);    
  //Serial.println(Mask,BIN);
}

void cycleOctave () {
  if (oct>octRange) oct=0;
  else oct++;
  return;
}
void cycleNote () {
  //note, it must be reviewed the ontes array in order to address notes with 7 semitones displacement, so it should be covering 19
  note=scale[noteIndex+majorChords[chordIndex]];
  if (chordIndex>=4) chordIndex=0;
  else chordIndex++;
  }
void getInput () {
   if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      switch (incomingByte) {
        case (97):          //a
          note=scale[0];
          noteIndex=0;
        break;
        case (119):
          note=scale[1];    //w
          noteIndex=1;
        break;
        case (115):         //b
          note=scale[2];    
          noteIndex=2;
        break;
        case (101):         //e
          note=scale[3];    
          noteIndex=3;          
        break;
        case (100):         //d
          note=scale[4];    
          noteIndex=4;          
        break;
        case (102):         //f
          note=scale[5];
          noteIndex=5;          
        break;
        case (116):         //t
          note=scale[6];
          noteIndex=6;          
        break;
        case (103):         //g
          note=scale[7];
          noteIndex=7;          
        break;
        case (121):         //y
          note=scale[8];
          noteIndex=8;          
        break;
        case (104):         //h
          note=scale[9];
          noteIndex=9;          
        break;        
        case (117):
          note=scale[10];   //u
          noteIndex=10;          
        break;
        case (106):         //j
          note=scale[11];
          noteIndex=11;          
        break;
        case (107):         //k
          note=scale[12];
          noteIndex=12;          
        break;
        case (113):         //q
          note=0;
        break;
        case (109):         //m
          if (oct<5) oct++;
        break;
        case (110):         //n
          if (oct>0) oct--;
        break;
        case (122):         //z
          tempo=tempo+5;
        break;        
        case (120):         //x
          tempo=tempo-5;
          if (tempo<30) tempo=30;
        break;
        case (99):          //c  
          octRange++;
          if (octRange>4) octRange=1;
        break;
        case (98):            //b (arpeggiator mode)
          if (arpMode>=2) arpMode=0;
          else arpMode++;
        break;
        case (49):            //1
                  waveforms=1;
                  Serial.print("wave="+String(waveforms));
        break;
        case (50):            //2
                  waveforms=2;
                  Serial.print("wave="+String(waveforms));
        break;
        case (51):            //3
                  waveforms=3;
                  Serial.print("wave="+String(waveforms));
        break;
        case (52):            //4
                  waveforms=4;
                  Serial.print("wave="+String(waveforms));
        break;
        case (53):            //5
                  waveforms=5;
                  Serial.print("wave="+String(waveforms));
        break;
        case (54):            //6
                  waveforms=6;
                  Serial.print("wave="+String(waveforms));
        break;
        }
        outFreq=note*oct_multi[oct]*20000/65536;
      Serial.print("freq: "+String(outFreq));
      Serial.println(incomingByte, DEC);
  }
}
void loop() {  
  if(((millis()-checktime)>tempo) and note!=0 and arpMode!=0)
      {
        switch (arpMode) {
          case(1): cycleNote();
          break;
          case(2): cycleOctave();
          break;
          }         
         checktime=millis();
        }
      getInput();  
   }
