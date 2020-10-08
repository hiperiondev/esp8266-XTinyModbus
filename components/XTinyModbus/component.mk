#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS += ./ ./Modbus ./Modbus/ModbusBase ./Modbus/ModbusRTUMaster ./Modbus/ModbusRTUSlave ./Modbus/Serial ./Port/esp8266
COMPONENT_SRCDIRS += ./Modbus ./Modbus/ModbusBase ./Modbus/ModbusRTUMaster ./Modbus/ModbusRTUSlave ./Modbus/Serial ./Port/esp8266