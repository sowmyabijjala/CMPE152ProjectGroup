################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Pcl4.cpp 

OBJS += \
./Pcl4.o 

CPP_DEPS += \
./Pcl4.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I"/Users/sowmyabijjala/eclipse-workspace/Asgn4Cpp" -I"/Users/sowmyabijjala/eclipse-workspace/Asgn4Cpp/target/generated-sources/antlr4" -I/Users/sowmyabijjala/ANTLR-4.7.2/antlr4-runtime -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


