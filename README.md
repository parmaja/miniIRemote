### miniIRemote ###
IR Remote reader/receiver into 32bit 

## Tested ##
 
Attiny85, Atmel328/Arduino IDE 1.6 with common remote controls

## Usage ##

Extract content into "miniIRemote" folder in "library" of arduino 
Set digital pin number in your setup()

```c++
	void setup() {
		miniIR::init(11); //set to pin 11
	}
```
	
## Arduino Example ##

```c++
  unsigned long code = miniIR::Read();
  if (code > 0) //there is code 
  {       
    Serial.print("Code for this button: ");
    Serial.println(code);
  }
  else
    Serial.println("No code");
```

