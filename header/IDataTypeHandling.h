#pragma once
#include<modbus/modbus.h>
#include <bitset>
#include<iostream>

class DataTypeHandling{
    public:
    int modbus_write_reg_anologOP(modbus_t*,int,int32_t);

    int modbus_write_reg_anologOP(modbus_t*,int,float);

    int modbus_write_reg_anologOP(modbus_t*,int,double);

    int modbus_read_reg_analogOP(modbus_t*,int,int&);

    int modbus_read_reg_analogOP(modbus_t*,int,float&);

    int modbus_read_reg_analogOP(modbus_t*,int,double&);

    int modbus_read_reg_analogIP(modbus_t*,int,int&);

    int modbus_read_reg_analogIP(modbus_t*,int,float&);

    int modbus_read_reg_analogIP(modbus_t*,int,double&);
};