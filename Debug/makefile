################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Drivers/STM32L4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := SolarTracker_STM32L412CBU6
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
SolarTracker_STM32L412CBU6.elf \

MAP_FILES += \
SolarTracker_STM32L412CBU6.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
SolarTracker_STM32L412CBU6.list \


# All Target
all: main-build

# Main-build Target
main-build: SolarTracker_STM32L412CBU6.elf secondary-outputs

# Tool invocations
SolarTracker_STM32L412CBU6.elf SolarTracker_STM32L412CBU6.map: $(OBJS) $(USER_OBJS) C:\Users\stefe\STM32CubeIDE\workspace_1.9.0\SolarTracker_STM32L412CBU6\STM32L412CBUX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "SolarTracker_STM32L412CBU6.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\stefe\STM32CubeIDE\workspace_1.9.0\SolarTracker_STM32L412CBU6\STM32L412CBUX_FLASH.ld" --specs=nosys.specs -Wl,-Map="SolarTracker_STM32L412CBU6.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

SolarTracker_STM32L412CBU6.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "SolarTracker_STM32L412CBU6.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) SolarTracker_STM32L412CBU6.elf SolarTracker_STM32L412CBU6.list SolarTracker_STM32L412CBU6.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
