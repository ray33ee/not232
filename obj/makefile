################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/comms/src/subdir.mk
-include User/VMIOX/src/subdir.mk
-include User/USB_Serial/src/subdir.mk
-include User/Features/src/subdir.mk
-include User/FAT/src/subdir.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
not232.hex \

SECONDARY_LIST += \
not232.lst \

SECONDARY_SIZE += \
not232.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: not232.elf secondary-outputs

# Tool invocations
not232.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -fdump-rtl-expand -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -T "e:\Software Projects\moun_projects\not232\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Xlinker --print-memory-usage -Wl,-Map,"not232.map" --specs=nano.specs --specs=nosys.specs -o "not232.elf" $(OBJS) $(LIBS)
not232.hex: not232.elf
	@	riscv-none-embed-objcopy -O ihex "not232.elf" "not232.hex"
not232.lst: not232.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "not232.elf" > "not232.lst"
not232.siz: not232.elf
	riscv-none-embed-size --format=berkeley "not232.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) not232.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets