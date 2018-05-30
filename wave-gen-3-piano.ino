unsigned int Acc;
byte accTopTemp=0;
byte noteIndex=0;
byte chordIndex=0;
byte Mask=0;
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
const uint16_t scale[] = {857,908,962,1020,1080,1144,1212,1285,1361,1442,1528,1618,1715};
const float oct_multi[] = {0.125,0.25,0.5,1,2,4};
const int majorChords[] = {0,4,7}; //semitone sequence 0+4+7

void setup() { 
  // put your setup code here, to run once:
pinMode (A0,INPUT);
Serial.begin(9600);

DDRD = DDRD | 1<<DDD3;                     // PD3 (Arduino D3) as output
DDRB = DDRB | 1<<DDB5; //pin13 


//use one timer T0 to select a sampling frequency @20Khz and trigger an interrupt
TCCR1A = 0<<COM1A0 | 0<<COM1B0 | 0<<WGM10 | 0<<WGM11; // CTC TOP=OCR1A
OCR1A = 99;
TCCR1B = 1<<WGM12 | 2<<CS10;  //prescaler 1:8 (16Mhz/8)/99+1=20Khz
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
      OCR2B=(Acc>>8)&0x80;        //square (but why it should be 128?!) potrei aggiungere un check e se != da 0 forzare a FF
    break;
    case (2):
      OCR2B=Acc>>8;               //sawtooth questo arriva a FF
    break;
    case (3):
      OCR2B=accTopTemp^Mask;      //triangle questo oscilla su 0-128-0 ancora una volta non raggiunge volume di fondo scala perchè?
    break;
    case (4):
      OCR2B=127*(1+sin(Acc));     //sine wave
    break;
    }
}

void cycleOctave () {
  if (oct>octRange) oct=0;
  else oct++;
  return;
}
void cycleNote () {
  //note, it must be reviewed the ontes array in order to address notes with 7 semitones displacement, so it should be covering 19
  note=scale[noteIndex+majorChords[chordIndex]];
  if (chordIndex>=2) chordIndex=0;
  else chordIndex++;
  }
  
void loop() {
   // send data only when you receive data:
   if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      switch (incomingByte) {
        case (97):
          note=scale[0];
          noteIndex=0;
        break;
        case (119):
          note=scale[1];
          noteIndex=1;
        break;
        case (115):
          note=scale[2];
          noteIndex=2;
        break;
        case (101):
          note=scale[3];
          noteIndex=3;          
        break;
        case (100):
          note=scale[4];
          noteIndex=4;          
        break;
        case (102):
          note=scale[5];
          noteIndex=5;          
        break;
        case (116):
          note=scale[6];
          noteIndex=6;          
        break;
        case (103):
          note=scale[7];
          noteIndex=7;          
        break;
        case (121):
          note=scale[8];
          noteIndex=8;          
        break;
        case (104):
          note=scale[9];
          noteIndex=9;          
        break;        
        case (117):
          note=scale[10];
          noteIndex=10;          
        break;
        case (106):
          note=scale[11];
          noteIndex=11;          
        break;
        case (107):
          note=scale[12];
          noteIndex=12;          
        break;
        case (113):
          note=0;
        break;
        case (109):
          if (oct<5) oct++;
        break;
        case (110):
          if (oct>0) oct--;
        break;
        case (122):
          tempo=tempo+30;
        break;        
        case (120):
          tempo=tempo-30;
          if (tempo<30) tempo=30;
        break;
        case (99):
          octRange++;
          if (octRange>4) octRange=1;
        break;
        case (118): // "v" waveform switch
          if (waveforms==4) waveforms=1;
          else waveforms++;
        break;
        }
      // say what you got:
      outFreq=note*oct_multi[oct]*20000/65536;
      Serial.print("freq: "+String(outFreq));
      Serial.println(incomingByte, DEC);
      
   }
 if((millis()-checktime)>tempo)
      {
        cycleOctave();
        checktime=millis();
        }
 }
 /*
     for (int i=0;i<12;i++)
    {
      if (i
      note=scale[i];
      Serial.println(note);
      delay(1000);
      note=0;
      delay(10);
      }
     note=0; 
 */
