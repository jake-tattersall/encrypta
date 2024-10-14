# Encrypta: The Modern Man's Enigma Machine
The idea for Encrypta was spawned by a late-night question fueled by little sleep and a gaming session: "How would a modern version of the Enigma machine work?" This is our answer.

### How does Encrypta work?

Encrypta utilizes the ESP-NOW communication protocol embedded within the ESP32 WROOM microcontrollers on which it is based. This protocol is peer-to-peer, wireless, and, most notably, unencrypted by default.

This gave us the opportunity to develop our own encryption method for our communicators, which emulates the functionality of the Enigma machine. In our system, a message is scrambled manually before being transmitted over open airwaves to the intended recipient, who can then decrypt it. Meanwhile, any spies or unwanted listeners remain unaware of what was sent.

The project started with assembling the hardware into a handheld prototype package, which can be powered by the USB port on a laptop or a power bank.

We chose 0.96" OLED screens from Adafruit due to their ability to display more than the average 16x2 LCD screen one would use in a regular Arduino kit. This has the advantage of allowing longer messages, which in turn utilizes more of the 250-byte limit on data transmission via ESP-NOW.

Encrypta uses a standard Playfair cipher. When the devices start up, the user is prompted to enter a keyword. If both users input the same pre-agreed keyword, the same key table will be generated for each device.

### What challenges did we run into?

We faced many challenges throughout this project, chief among them being the quirks of the hardware itself. Setting up a wireless connection between two boards none of us had ever used before was no easy task, and neither was getting the encryption/decryption itself to work before transmitting. All of this was on top of cable management and very little sleep.

### What does the future hold for Encrypta?

In the future, we would like to expand Encrypta's capabilities by using a custom-made transmission and receiving protocol, as well as a small antenna, like the ones found on walkie-talkies, to increase the overall range of Encrypta. One feature discussed for future implementation was to allow Encrypta to convert the final encrypted string into Morse code. This could be hooked up to a radio handset like a hand key and use voltage pulses corresponding to shorts (.) and longs (-), which could then be translated by the receiving end into the encrypted message and decrypted for the receiver to read.
