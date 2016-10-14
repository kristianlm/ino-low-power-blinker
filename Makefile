
# this uses the Arduino IDE to build. It's not great (spits our error
# messages in dialog boxes, not stderr and is very un-command-liny but
# it seems to work.)

# to support the attiny85 for arduino, follow this guide:
# http://highlowtech.org/?p=1695


p = attiny85

./build/sleep.ino.hex: sleep/sleep.ino
	arduino \
		--board attiny:avr:ATtinyX5:cpu=attiny85 \
		--pref build.path=./build \
		--verify \
		sleep/sleep.ino

upload:
	sudo avrdude -p$p -cavrispmkII -Uflash:w:build/sleep.ino.hex:i
