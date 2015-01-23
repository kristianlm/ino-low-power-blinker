p := attiny85

all: compile upload

compile:
	ino build

upload:
	sudo avrdude -p$p -cavrispmkII -Uflash:w:.build/$p-8/firmware.hex:i
