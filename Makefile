DUMP_OFFSET := 0x08100000
FIRMWARE_SIZE := 6444

prog:
	openocd \
		-f interface/stlink.cfg \
		-f target/stm32h7x.cfg \
		-c init \
		-c "dump_image ./dump.bin $(DUMP_OFFSET) $(FIRMWARE_SIZE)" \
		-c exit
