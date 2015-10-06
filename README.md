# EZBlueSmirfUART
Sometimes, you just want two Arduinos to talk to each other. This code makes creating a secure, paired UART really fast and easy with the BlueSmirf Bluetooth controller. Surprisingly this simply did not exist at the time this library was written.

This library has been tested on the BlueSmirf GOLD which an be found here:
https://www.sparkfun.com/products/12582

However, it should work on all other Bluesmirf projects.


Why is this library so awesome? Because it does exactly what you'd want bluetooth to do: connect a serial wire over the air with little fuss.

Here's what a programmer expects:

1. Buys two bluetooth modules
2. Knows the mac address of both modules because it's printed on the sticker.
3. Two arduinos are loaded with this library with the mac addresses of each bluetooth module.
4. The arduinos find each other and then connect
5. UART style communication using the standard Serial.print() and .read() magically work!

The reason why this hasn't existed before as a discrete library is because the Bluetooth standard is complex interfacing with the bluetooth module requires a certain sequence of ASCII characters sent and read in order to set the state of the product. The existing tutorials on Sparkfun tell you how to connect with your computer, but that has limited use. I personally wanted to replace the serial connection wire and this required that I read through an 83 page manual located here:
https://cdn.sparkfun.com/assets/1/e/e/5/d/5217b297757b7fd3748b4567.pdf

The presented solution code will do the following:
1. Reset the bluetooth module on startup to factory settings.
2. Set encryption, pincode, the address of the other device, and the connection type (bluetooth pairing).
3. Reboot device to commit changes.
4. Device will now pair and is ready for data!

An example sketch will be as simple as:


    // Use hardware or software serial. In this instance we use Seria1, which is pin 0 & 1 on
    // teensy LC. DO NOT SET AS SERIAL or bad things will happen.
    #define Bluetooth Serial1
    //#define BT_NOVERBOSE
    
    // Including this file will pull in Bluetooth.
    #include "EZBlueSmirfUART.h"
    
    void setup() {
      Serial.begin(9600);
      BT_BondPrivatelyWith("00066673E605", "00066673E631");
    }
    
    void loop() {
      BT_Echo();
    }
    
There is a couple of things to notice, the BT_BondPrivatelyWith(...) takes BOTH bluetooth mac addresses. This is useful for enhanced error detection. It also allows the same exact sketch to be used on BOTH arduinos without change.
