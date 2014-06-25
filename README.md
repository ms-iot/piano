#Piano on Galileo

### Components
- Intel Galileo Board
- Windows Image on microSD card
- MP3 Player Shield
- GPIO Port Expander Chip
- 14 LEDS

### How The Code is Broken Up
- Main.cpp
- KeyboardLightController.cpp and .h
- PianoLogic.cpp and .h
- SpiMidi.cpp and .h
- spi_missing.h
- stdafx.h

#### Main
- The main function that initializes the Galileo Pins to talk SPI to the Music Shield and reads input from the keyboard in order to trigger the sounds and lights
#### PianoLogic
- Houses all of the logic for the piano system such as keyUp, keyDown, Lighting, Playing a sound, reading the XML files, etc.
#### KeyboardLightController
- Houses all of the functionality to trigger the lighting through the GPIO port expander
#### spi_missing
- Creates a namespace that hosts the functions missing from spi.h such as begin, end, and transfer with options. 
#### SpiMidi
- Houses all of the MIDI commands through SPI in order to simplify calling code
#### stdafx
- Houses all of the external includes for the project

### Configuration Files
- KeyMap.xml
    - Xml that contains the mapping for a keyboard key to a MIDI note and the Light position
    - Also contains mapping of keyboard keys to special functions like AutoPlay
- Song.xml
    - The song to be auto-played
    - Each note in the song is composed of the keyboard key to be "pressed" and the how long it should be pressed (duration)
    - Since 'y' was not linked to a note, we used it here for a wait between notes