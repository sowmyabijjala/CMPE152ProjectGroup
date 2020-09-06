################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/frontend/Parser.cpp \
../src/frontend/Token.cpp 

OBJS += \
./src/frontend/Parser.o \
./src/frontend/Token.o 

CPP_DEPS += \
./src/frontend/Parser.d \
./src/frontend/Token.d 


# Each subdirectory must supply rules for building sources it contributes
src/frontend/%.o: ../src/frontend/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


