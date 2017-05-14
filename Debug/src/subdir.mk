################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/base64.cpp \
../src/jsoncpp.cpp \
../src/test_baidu_vrc.cpp 

OBJS += \
./src/base64.o \
./src/jsoncpp.o \
./src/test_baidu_vrc.o 

CPP_DEPS += \
./src/base64.d \
./src/jsoncpp.d \
./src/test_baidu_vrc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I"/home/zulolo/workspace/test_baidu_vrc/include" -I/home/zulolo/lib4orangepi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


