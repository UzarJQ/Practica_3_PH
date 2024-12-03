################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../8led.c \
../button.c \
../cola.c \
../led.c \
../main.c \
../sudoku_2024.c \
../timer.c 

OBJS += \
./8led.o \
./button.o \
./cola.o \
./led.o \
./main.o \
./sudoku_2024.o \
./timer.o 

C_DEPS += \
./8led.d \
./button.d \
./cola.d \
./led.d \
./main.d \
./sudoku_2024.d \
./timer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -I"C:\Users\Jorge\Eclipse-Juno\Practica_2_PH\common" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


