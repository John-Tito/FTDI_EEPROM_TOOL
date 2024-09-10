# ftdi_dumps
读取旧固件

.\EEPROM_TOOLS.exe -r -o old.bin

修改旧固件中序列号部分，除序列号之外其他部分不可改变，数据总长度不可改变

EEPROM_TOOLS.exe -w -i NEW.BIN