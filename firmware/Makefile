# Project: Smart Power Bank Keep-Alive
# Author: Zak Kemble, contact@zakkemble.net
# Copyright: (C) 2020 by Zak Kemble
# License: 
# Web: https://blog.zakkemble.net/smart-power-bank-keep-alive/

PROJECT=spbka

MCU=attiny10

F_CPU=4000000UL
CPU_DIV=clock_div_2

# COMPILER is passed from runmake.bat
ifndef COMPILER
COMPILER=
endif

# AVRDUDE is passed from runmake.bat
ifndef AVRDUDE
AVRDUDE=avrdude
endif

PREFIX=avr-

SRC_DIR=.
INC_DIR=.
OBJ_DIR=obj
BIN_DIR=bin

FILES= \
	main.c

CFLAGS= \
	-mmcu=$(MCU) \
	-c \
	-x c \
	-Wall \
	-Wextra \
	-Wstrict-prototypes \
	-Wunused-result \
	-std=gnu99 \
	-O1 \
	-mcall-prologues \
	-funsigned-char \
	-funsigned-bitfields \
	-fno-tree-scev-cprop \
	-ffreestanding \
	-fno-exceptions \
	-maccumulate-args \
	-ffunction-sections \
	-fdata-sections \
	-fpack-struct \
	-fshort-enums \
	-mrelax \
	-g2 \
	-I$(INC_DIR) \
	-fno-tree-loop-optimize \
	-fno-caller-saves

LDFLAGS= \
	-Wl,-Map="$(BIN_DIR)\$(PROJECT).map" \
	-Wl,--start-group \
	-Wl,--end-group \
	-Wl,--gc-sections \
	-Wa,-g \
	-mrelax \
	-mmcu=$(MCU)

DEFS= \
	-DF_CPU=$(F_CPU) \
	-DCPU_DIV=$(CPU_DIV)

LDLIBS= 

DEPFLAGS= \
	-MD -MP -MT "$(@:%.o=%.d)" -MT "$@"

# https://gcc.gnu.org/onlinedocs/cpp/Invocation.html
# http://scottmcpeak.com/autodepend/autodepend.html

ELF=$(PROJECT).elf
HEX=$(PROJECT).hex
EEP=$(PROJECT).eep
LSS=$(PROJECT).lss

CC=$(COMPILER)$(PREFIX)gcc
LD=$(COMPILER)$(PREFIX)gcc
OBJCOPY=$(COMPILER)$(PREFIX)objcopy
OBJDUMP=$(COMPILER)$(PREFIX)objdump
SIZE=$(COMPILER)$(PREFIX)size

SOURCES=$(FILES:%.c=$(SRC_DIR)/%.c)
OBJECTS=$(FILES:%.c=$(OBJ_DIR)/%.o)

all: $(SOURCES) $(ELF)

$(ELF): $(OBJECTS)
	@echo Linking...
	@mkdir "$(BIN_DIR)" 2>nul || true
	@$(LD) $(LDFLAGS) $(OBJECTS) -o $(BIN_DIR)/$@ $(LDLIBS)
	@$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures $(BIN_DIR)/$(ELF) $(BIN_DIR)/$(HEX)
	@$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex $(BIN_DIR)/$(ELF) $(BIN_DIR)/$(EEP)
	@$(OBJDUMP) -h -S $(BIN_DIR)/$(ELF) > $(BIN_DIR)/$(LSS)
	@echo .
	@echo .
	@$(SIZE) $(BIN_DIR)/$(HEX) $(BIN_DIR)/$(ELF)
	@echo .
	@echo .

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)/%.d Makefile
	@echo Compiling $<...
	@mkdir "$(dir $@)" 2>nul || true
	@$(CC) $(DEPFLAGS) $(DEFS) $(CFLAGS) $< -o $@

clean:
	@rm -rf $(OBJ_DIR)/* $(BIN_DIR)/* 2>nul || true

upload:
	$(AVRDUDE) -c usbasp -B 16.0 -p $(MCU) -U flash:w:$(BIN_DIR)/$(HEX)

$(OBJ_DIR)/%.d: ;

.PHONY: clean all upload

-include $(patsubst %,$(OBJ_DIR)/%.d,$(basename $(FILES)))
