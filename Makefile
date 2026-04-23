SRC_DIR:=src
INC_DIR:=inc
CFG_DIR:=cfg

CURRENT_USER:=$(shell whoami)
CMSIS_DIR:= \
	-I/home/$(CURRENT_USER)/.platformio/packages/framework-cmsis-stm32h7/Include \
	-I/home/$(CURRENT_USER)/.platformio/packages/framework-cmsis/CMSIS/Core/Include

LD_FILE:=link.ld
OUT_FILE:=project.elf


# STLINK_CFG_FILE:=/usr/share/openocd/scripts/interface/stlink.cfg
# STM32_CFG_FILE:=/usr/share/openocd/scripts/target/stm32h7x.cfg
STLINK_CFG_FILE :=$(CFG_DIR)/interface/stlink.cfg
STM32_CFG_FILE  :=$(CFG_DIR)/target/stm32h7x.cfg

# Toolchain
TC:=arm-none-eabi-
CC:=$(TC)gcc
LD:=$(TC)ld
SIZE:=$(TC)size
CFLAGS:=-c -g -mthumb -mcpu=cortex-m7 --specs=nosys.specs -O0 -std=gnu17 \
	-I$(INC_DIR) \
	$(CMSIS_DIR) \
	-D CORE_CM7 \
	-D STM32H745xx

LDFLAGS:= -T $(LD_FILE) -o $(OUT_FILE)


OCD_TOOL:=openocd
OCD_OPTS:= \
	-f $(STLINK_CFG_FILE) \
	-f $(STM32_CFG_FILE) \
	-c init \
	-c "reset halt" \
	-c "flash write_image erase $(OUT_FILE)" \
	-c "verify_image $(OUT_FILE)" \
	-c reset \
	-c exit

SRC_FILES:=$(wildcard $(SRC_DIR)/*.c)
OBJ_FILES:=$(SRC_FILES:$(SRC_DIR)/%.c=%.o)

.PHONY: all clean prog rebuild debug connect

all: $(OUT_FILE)

# Build object files
%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

# Build executable file
$(OUT_FILE): $(OBJ_FILES)
	$(LD) $(LDFLAGS) $(OBJ_FILES)
	echo "Successful build!"
	arm-none-eabi-objcopy -O binary $(OUT_FILE) project.bin
	$(SIZE) $(OUT_FILE)
	

rebuild: clean $(OUT_FILE)

prog: $(OUT_FILE)
	$(OCD_TOOL) $(OCD_OPTS)

clean:
	rm -f $(OBJ_FILES) $(OUT_FILE) project.bin

connect: prog
	$(OCD_TOOL) -f $(STLINK_CFG_FILE) -f $(STM32_CFG_FILE) -c init -c "reset halt"

debug:
	arm-none-eabi-gdb $(OUT_FILE) -ex "target extended-remote localhost:3333"