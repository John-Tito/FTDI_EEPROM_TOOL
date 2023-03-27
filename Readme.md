EEPROM_TOOLS
===========
# introduction
A simple command line program that reads data from or writes data to the EEPROM of FTDI's FT4232H USB-to-serial interface chip and automatically performs checksum calculations. This program is not guaranteed to run on FT4232H and when the EEPROM size is 128 bytes, modify the source code yourself if needed.

# usage
> USAGE : EEPROM_TOOLS.exe [-r [-o <file name>]] [-w [-i <file name>]] [-h]
>
> read data from eeprom or write data to eeprom of ftdi chip
>
> Options:
>
> -r, --read           read data from eeprom
>
> -o, --output_file    output file name, \"old.bin\" by default
>
> -w, --write          write data to eeprom
>
> -i, --input_file     input file name, \"new.bin\" by default
>
> -h, --help           Print this help screen and exit

# additional notes
The CLI part is forked from : [nemequ/ftdi-eeprom](https://github.com/nemequ/ftdi-eeprom)

report BUG : [John-Tito](https://github.com/John-Tito/FTDI_EEPROM_TOOL)
