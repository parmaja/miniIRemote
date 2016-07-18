/**
*   miniIRemote, read IR sensor pulses reader return a code (uint32).
*
* @license   The MIT License (MIT) 
*
* @ref:
*    
* @author:    Zaher Dirkey <zaherdirkey at yahoo dot com>
* @usage:
*
* unsigned long irNumber = miniIR::Read();
* if (irNumber > 0) { your code }
*
* TODO:
*   Skip bit 
*   Reverse bits
*
*/

//I keep it for debugging propose 

//Attiny:
/*
#define IRPort PINB
#define IRPin 0
#define getPulse       (IRPort & _BV(IRPin))
*/
//Arduino:
/*
#define IRPort         PINB
#define IRPin          3
#define getPulse       (IRPort & _BV(IRPin))
*/

//#define getPulse      digitalRead(irPin) //slow, not work

//* DIRECT_PIN_READ ported from https://github.com/PaulStoffregen/Encoder/blob/master/utility/direct_pin_read.h#L7
#define DIRECT_PIN_READ(base, mask) (((*(base)) & (mask)) ? 1 : 0)
#define getPulse      DIRECT_PIN_READ(irPort, irPinMask)

//#define DEBUG //enable it to send out info over Serial, not work in attiny, you can define it in your project

namespace miniIR 
{
#ifdef USB_CFG_IOPORTNAME //check if vusb used, to call usbPoll()
#define VUSB
#endif

#define IR_PULSE_COUNT 32 // IR pulses count 
#define IR_MINPULSES 4 //min acceptable count, return code 0 if less than 4
#define IR_RESOLUTION 2 
#define IR_MAX_LENGTH 5000 //we dont need to not wait forever, at least need to call usbPoll()

//UNIVERSAL the most common remote (i have :P)
#define IR_DATA_LENGTH 600
#define IR_LO_DATACOME 1500 //Skip first data
#define IR_LONG_LENGTH 2500

//SONY , used Lo pulses but universal above works fine
/*#define IR_DATA_LENGTH 600 //higher=1 lesser = 0
#define IR_LO_DATACOME 1500 //not sure
#define IR_LONG_LENGTH 2500
*/

/*
//FAN, using low bit, just my fan remote control, ignore it 
#define IR_DATA_LENGTH 300
#define IR_LO_DATACOME 0
#define IR_LONG_LENGTH 2500
*/
    uint8_t irDigitalPin = -1;
	uint8_t irPinMask = 0;
	volatile uint8_t* irPort = 0;
    
    PROGMEM void init(uint8_t pin)
    {
        irDigitalPin = pin;        
		irPort = portInputRegister(digitalPinToPort(irDigitalPin));
		irPinMask = digitalPinToBitMask(irDigitalPin);
        pinMode(irDigitalPin, INPUT); // Set IR pin as input
    }
    
    #ifdef DEBUG
    uint16_t maxLength = 0;
    uint16_t minLength = 0;
    #endif 

    PROGMEM uint32_t Read()
    {  
      uint16_t lo_pulse =0, hi_pulse =0;
      
      struct {
        bool started = false;
        uint32_t code = 0;
        uint8_t count = 0; 
        #ifdef DEBUG
        bool b = false;
        #endif
        
        void reset(){
          #ifdef DEBUG
          b = false;
          maxLength = 0;
          minLength = 0;
          #endif 
          
          count = 0;
          code = 0;
        }

        void add(uint16_t pulse)
        {   
          code = code << 1;
          if (pulse > IR_DATA_LENGTH){
            code = code | 1;      
          }      
          count++;
           
          #ifdef DEBUG
          if ((pulse > 25) and (count > 1)) { //skipped first one for fine values
            if (!b || (maxLength < pulse))
              maxLength = pulse;
            if (!b || (minLength > pulse))
              minLength = pulse;
            b = true;
          }
          #endif 
        }
      } data;

    /*
     * Starting loop to collect pulses 
     * First pulse it ignored, it was high, waiting forever
     * after lo pulse come usually it is long low pulse, used to sign of data comming
     * second high maybe it is a long for data comming sign too, we can ignore it, sony not need it 
     * 
     */
      data.reset();
      
      while (data.count <= IR_PULSE_COUNT)
      {
        #ifdef VUSB
        usbPoll(); 
        #endif

        hi_pulse = 0;
        while (getPulse) { //Wait for ever for the first pulse
          hi_pulse++;
          delayMicroseconds(IR_RESOLUTION);      
          if (hi_pulse >= IR_LONG_LENGTH)
          {
            if (data.count > 0)
            {
              #ifdef DEBUG
              Serial.print("Long Length");        
              #endif 
              if (!data.started)
                return 0;
              else 
              {
				#ifdef DEBUG
				Serial.print(" Count= ");
				Serial.println(data.count);
				#endif 
				return data.code;
              }
            }
            else if (hi_pulse >= IR_MAX_LENGTH) 
            {
              hi_pulse = 0;
              #ifdef VUSB
              usbPoll(); 
              #endif
              //continue;
            }
          }
        }
        lo_pulse = 0;
        while (!getPulse) {
          lo_pulse++;
          delayMicroseconds(IR_RESOLUTION);
          if ((lo_pulse >= IR_MAX_LENGTH)) {
            #ifdef DEBUG
            Serial.println(" lo pulse IR_MAX_LENGTH ");        
            #endif 
            #ifdef VUSB
            usbPoll(); 
            #endif
            
            return 0; //too long data length
          }
        }

        if (IR_LO_DATACOME > 0)
        {
            if (!data.started) {
              data.reset();
              data.started = true;
            }
            if ((lo_pulse >= IR_LO_DATACOME))
              continue;
        }
        else {
            if (!data.started) {
              data.reset();
              data.started = true;
            }
        }
        
        if (data.started) {
          data.add(hi_pulse + lo_pulse);
        }
      }

    #ifdef DEBUG
      Serial.println("Exit");
    #endif
      return data.code;
    }
} //end of namespace miniIR