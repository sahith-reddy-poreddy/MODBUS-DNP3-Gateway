#pragma once

#include <modbus/modbus.h>
#include <opendnp3/DNP3Manager.h>
#include <opendnp3/channel/PrintingChannelListener.h>
#include <opendnp3/logging/LogLevels.h>
#include <opendnp3/outstation/DefaultOutstationApplication.h>
#include <opendnp3/outstation/IUpdateHandler.h>
#include <opendnp3/outstation/SimpleCommandHandler.h>
#include <opendnp3/outstation/UpdateBuilder.h>
#include <opendnp3/outstation/ICommandHandler.h>
#include <iostream>
#include <unordered_map>
#include <IDataBaseHandler.h>


using namespace std;

using namespace opendnp3;

class MyCommandHandler : public ICommandHandler
{
public:

    modbus_t *slv;        

    DataMap mapping;

    virtual void Begin() override;

    virtual void End() override;

    virtual CommandStatus Select(const ControlRelayOutputBlock &command, uint16_t index) override;

    virtual CommandStatus Operate(const ControlRelayOutputBlock &command, uint16_t index, IUpdateHandler &handler, OperateType opType) override;

    virtual CommandStatus Select(const AnalogOutputInt16 &command, uint16_t index) override;

    virtual CommandStatus Operate(const AnalogOutputInt16 &command,uint16_t index,IUpdateHandler &handler,OperateType opType)override;

    virtual CommandStatus Select(const AnalogOutputInt32 &command, uint16_t index) override;

    virtual CommandStatus Operate(const AnalogOutputInt32 &command,uint16_t index,IUpdateHandler &handler,OperateType opType)override;

    virtual CommandStatus Select(const AnalogOutputFloat32 &command, uint16_t index) override;

    virtual CommandStatus Operate(const AnalogOutputFloat32 &command,uint16_t index,IUpdateHandler &handler,OperateType opType)override;

    virtual CommandStatus Select(const AnalogOutputDouble64 &command, uint16_t index) override;

    virtual CommandStatus Operate(const AnalogOutputDouble64 &command,uint16_t index,IUpdateHandler &handler,OperateType opType)override;
};
