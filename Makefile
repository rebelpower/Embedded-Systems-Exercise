MCU   = atmega328p
F_CPU = 16000000
BAUD_SERIAL = 250000

# MCU = atmega64
# F_CPU = 12000000UL
# BAUD_SERIAL = 9600

# PROGRAMMER_ARGS = -P /dev/ttyACM1
PROGRAMMER_ARGS = -P COM3
# PROGRAMMER_ARGS_STK = -P /dev/ttyACM1
SERIAL_PORT_DEBUG = COM7
# SERIAL_PORT_DEBUG_STK500v2 = /dev/ttyUSB0

BAUD = $(BAUD_SERIAL)UL

#################################################
# classical/standard compilation options Target
#################################################
CPPFLAGS = -DF_CPU=$(F_CPU) -DBAUD=$(BAUD)
CFLAGS = -O2 -g2 -gstabs -std=c99 -Wall
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
# CFLAGS += -funsigned-char -funsigned-bitfields -fshort-enums
CFLAGS += -MMD -MP
CFLAGS += -ffunction-sections -fdata-sections -MT"$@" -MF"$(@:.o=.d)"

#################################################
# classical/standard compilation options Local
#################################################
CFLAGS_LOCAL = -O2 -g2 -gstabs -std=c99 -Wall
CFLAGS_LOCAL += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
# CFLAGS += -funsigned-char -funsigned-bitfields -fshort-enums
CFLAGS_LOCAL += -MMD -MP
CFLAGS_LOCAL += -ffunction-sections -fdata-sections -MT"$@" -MF"$(@:.o=.d)"


#################################################
# User specified flags
# Add your own flags here, to your fullest content
#
#################################################

# CFLAGS += -DINTEGRATION_BME680
# CFLAGS += -DINTEGRATION_MOISTURE
# CFLAGS += -DBME680_FLOAT_POINT_COMPENSATION   #so that i can avoid the floatingpoint shite
# CFLAGS += -DINTEGRATION_POTATO
# CFLAGS += -DINTEGRATION_PLANTSENSOR
# CFLAGS += -DINTEGRATION_TEMP


###############################################
###############################################
###############################################

# Linker flags target
LDFLAGS = -Wl,-Map,$(TARGET).map
LDFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm
TARGET_ARCH = -mmcu=$(MCU)

# Linker flags local
LDFLAGS_LOCAL = -Wl,-Map,$(TARGET).map
LDFLAGS_LOCAL += -Wl,-u,-lm

#Directories
CEEDLING_PROJECT_PREFIX := project
BUILD_DIR			:= buildDir
BUILD_DIR_LOCAL		:= buildDirLocal
CURRENT_DIR_PATH	:= $(CURDIR)
CURRENT_DIR_NAME	:= $(notdir $(CURRENT_DIR_PATH))
CEEDLING_FOLDER		:= $(CEEDLING_PROJECT_PREFIX)_$(CURRENT_DIR_NAME)
SRC_DIR = $(CEEDLING_FOLDER)/src
#################################################
# Platform compilers Target 
#################################################
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size
AVRDUDE = avrdude

#################################################
# Local compilers
#################################################
CC_LOCAL = gcc

#################################################
# Build a list of directories under src
# finds all source folders starting at src
#################################################
SOURCE_DIRS := $(shell find $(SRC_DIR) -type d)

#used to create the list of .o files that is required to define as dependencies for any link target
#the sources in this list have their relative directory path as prefix, i.e. src/subdir/myFile.c
#This automatically adds all .c files up to the specified depth of subfolders. To increase, add another /* part
#  to the list after the wildcard, e.g. for three layers deep: $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c $(SRC_DIR)/*/*/*.c)
SOURCES = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c $(SRC_DIR)/*/*/*.c $(SRC_DIR)/*/*/*/*.c)

#this creates a list of .o files with the same path prefix as the SOURCES, i.e. src/subdir/myFile.o
RAW_OBJECT_FILES_WITH_DIRS = $(SOURCES:.c=.o)

#this cuts off the directory prefix and leaves only the filename, i.e. myFile.o
RAW_OBJECT_FILES_WITHOUT_DIR = $(notdir $(RAW_OBJECT_FILES_WITH_DIRS))

#This makes a list of all .o files that should be in the BUILD_DIR, i.e. buildDir/myFile.o
#This is used to create the dependencies for a link target, but not to actually compile them, as you can't pattern match .c to .o file.
OBJECT_FILES = $(addprefix $(BUILD_DIR)/, $(RAW_OBJECT_FILES_WITHOUT_DIR))

# For local runs
OBJECT_FILES_LOCAL = $(addprefix $(BUILD_DIR_LOCAL)/, $(RAW_OBJECT_FILES_WITHOUT_DIR))

#Use the list in $(SOURCE_DIRS) as a search path for .c files.
#This adds all .c files to that %.c thing from the list of all folders that are done by the SOURCE_DIRS
#With this you can simply do the %.o : %.c thing, as all necessary sources are well known to the %.c "variable (?)" <- not sure how much that makes sense
vpath %.c $(SOURCE_DIRS)

.PHONY: test clean all

test :
	cd $(CEEDLING_FOLDER) ; ceedling ; cd ..

all : clean release

# Target release build
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)/$(SOURCES_DIRS) 
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	 # $(OBJCOPY) -j .text -j .data -O ihex $< $@
	$(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O ihex $< $@
	# $(OBJCOPY) -j .text -j .data -O ihex $< $@

$(BUILD_DIR)/%.eeprom: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

$(BUILD_DIR)/%.lst: $(BUILD_DIR)/%.elf
	$(OBJDUMP) -h -S $< > $@

$(BUILD_DIR)/deployment.elf : $(OBJECT_FILES)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ -o $@


###############################################
# Local "release" build deps
###############################################

$(BUILD_DIR_LOCAL)/%.o: %.c | $(BUILD_DIR_LOCAL)/$(SOURCES_DIRS) 
	$(CC_LOCAL) $(CFLAGS_LOCAL) -c -o $@ $<

$(BUILD_DIR_LOCAL)/localTest : $(OBJECT_FILES_LOCAL)
	$(CC_LOCAL) $(LDFLAGS_LOCAL) $^ -o $@

makeExecutable:
	chmod +x $(BUILD_DIR_LOCAL)/localTest

###############################################
# 
###############################################

clean : 
	rm -rf $(BUILD_DIR) ; \
	rm -rf $(BUILD_DIR_LOCAL); \

compile : $(BUILD_DIR)/$(SOURCES_DIRS) | $(BUILD_DIR)/deployment.elf

compileLocal :  $(BUILD_DIR_LOCAL)/$(SOURCES_DIRS) | $(BUILD_DIR_LOCAL)/localTest

runLocal: compileLocal | makeExecutable
	$(BUILD_DIR_LOCAL)/localTest

createProjectFolder:
	mkdir -p $(CEEDLING_PROJECT_PREFIX)_$(CURRENT_DIR_NAME)/src

size: $(BUILD_DIR)/deployment.elf
	$(AVRSIZE) -C --mcu=$(MCU) $<

programWiring : $(BUILD_DIR)/deployment.hex
	$(AVRDUDE) -c wiring -p $(MCU) $(PROGRAMMER_ARGS) -D -V -U flash:w:$<

programArduino : $(BUILD_DIR)/deployment.hex
	$(AVRDUDE) -c arduino -p $(MCU) $(PROGRAMMER_ARGS) -D -V -U flash:w:$<

programStk500v2 : $(BUILD_DIR)/deployment.hex
	$(AVRDUDE) -c stk500v2 -p $(MCU) -V $(PROGRAMMER_ARGS_STK) -U flash:w:$<

debugWiring : programWiring
	python serial_echo.py $(SERIAL_PORT_DEBUG) $(BAUD_SERIAL)

debugArduino: programArduino
	python serial_echo.py $(SERIAL_PORT_DEBUG) $(BAUD_SERIAL)

debugStk500v2 : programStk500v2
	python serial_echo.py $(SERIAL_PORT_DEBUG_STK500v2) $(BAUD_SERIAL)

runSerial: 
	python serial_echo.py $(SERIAL_PORT_DEBUG) $(BAUD_SERIAL)

$(BUILD_DIR)/$(SOURCES_DIRS) :
	mkdir -p $@

$(BUILD_DIR_LOCAL)/$(SOURCES_DIRS) :
	mkdir -p $@	
