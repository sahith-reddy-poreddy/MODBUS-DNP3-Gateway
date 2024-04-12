#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <modbus/modbus.h>
#include <error.h>
#include <opendnp3/ConsoleLogger.h>
#include <opendnp3/DNP3Manager.h>
#include <opendnp3/channel/PrintingChannelListener.h>
#include <opendnp3/logging/LogLevels.h>
#include <opendnp3/outstation/DefaultOutstationApplication.h>
#include <opendnp3/outstation/IUpdateHandler.h>
#include <opendnp3/outstation/SimpleCommandHandler.h>
#include <opendnp3/outstation/UpdateBuilder.h>
#include <thread>
#include <fstream>
#include <atomic>
#include <opendnp3/outstation/ICommandHandler.h>
#include <unordered_map>

using namespace std;
using namespace opendnp3;

class MyCommandHandler : public opendnp3::ICommandHandler
{ // command handler handles incomming requests
public:
    modbus_t *slv;                   // modbus slave that is connected to dnp3 server
    unordered_map<int, int> mapping; // mapping of dnp3 to modbus to handle incomming requests
    virtual void Begin() override
    {
        cout << "MyCommandHandler called\n";
    }
    virtual void End() override
    {
        cout << "MyCommandHandler end\n";
    }

    virtual opendnp3::CommandStatus Select(const opendnp3::ControlRelayOutputBlock &command, uint16_t index) override
    {

        return opendnp3::CommandStatus::SUCCESS;
    }

    virtual opendnp3::CommandStatus Operate(const opendnp3::ControlRelayOutputBlock &command, uint16_t index, IUpdateHandler &handler, opendnp3::OperateType opType) override
    {

        handler.Update(BinaryOutputStatus(command.count), index);
        return opendnp3::CommandStatus::SUCCESS;
    }

    virtual CommandStatus Select(const AnalogOutputInt16 &command, uint16_t index) override
    {

        return opendnp3::CommandStatus::SUCCESS;
    }

    virtual CommandStatus Operate(const AnalogOutputInt16 &command,
                                  uint16_t index,
                                  IUpdateHandler &handler,
                                  OperateType opType)
        override
    {
        modbus_write_register(slv, mapping[index], command.value); // writing to modbus
        handler.Update(AnalogOutputStatus(command.value), index);  // writing to DNP3 server
        return opendnp3::CommandStatus::SUCCESS;
    }

    virtual CommandStatus Select(const AnalogOutputInt32 &command, uint16_t index) override
    {
        return opendnp3::CommandStatus::SUCCESS;
    }
    virtual CommandStatus Operate(const AnalogOutputInt32 &command,
                                  uint16_t index,
                                  IUpdateHandler &handler,
                                  OperateType opType)
        override
    {
        handler.Update(AnalogOutputStatus(command.value), index);
        return opendnp3::CommandStatus::SUCCESS;
    }
    virtual CommandStatus Select(const AnalogOutputFloat32 &command, uint16_t index) override
    {

        return opendnp3::CommandStatus::SUCCESS;
    }
    virtual CommandStatus Operate(const AnalogOutputFloat32 &command,
                                  uint16_t index,
                                  IUpdateHandler &handler,
                                  OperateType opType)
        override
    {
        handler.Update(AnalogOutputStatus(command.value), index);
        return opendnp3::CommandStatus::SUCCESS;
    }
    virtual CommandStatus Select(const AnalogOutputDouble64 &command, uint16_t index) override
    {
        return opendnp3::CommandStatus::SUCCESS;
    }

    virtual CommandStatus Operate(const AnalogOutputDouble64 &command,
                                  uint16_t index,
                                  IUpdateHandler &handler,
                                  OperateType opType)
        override
    {
        handler.Update(AnalogOutputStatus(command.value), index);
        return opendnp3::CommandStatus::SUCCESS;
    }
};

void csvhandling(unordered_map<int, int> &r_only, unordered_map<int, int> &r_and_w, unordered_map<int, int> &dnp3)
{ // reads the csv file to handle the mapping
    ifstream fout;
    fout.open("data_mapping.csv");
    string val;
    int a, b;
    while (getline(fout, val, ','))
    {
        if (val == "1")
        {
            getline(fout, val, ',');
            a = stoi(val);
            getline(fout, val);
            b = stoi(val);
            r_only[a] = b;
        }
        else
        {
            getline(fout, val, ',');
            a = stoi(val);
            getline(fout, val);
            b = stoi(val);
            r_and_w[a] = b;
            dnp3[b] = a;
        }
    }
}

DatabaseConfig ConfigureDatabase() // returns the default database setting for DNP3 server
{
    DatabaseConfig config(10);

    config.analog_input[0].clazz = PointClass::Class2;
    config.analog_input[0].svariation = StaticAnalogVariation::Group30Var5;
    config.analog_input[0].evariation = EventAnalogVariation::Group32Var7;

    return config;
};

auto app = DefaultOutstationApplication::Create(); // outstation/server application

std::atomic<bool> stopFlag(false); // flag to stop the exicution of modbus_read_a

void modbus_read_a(IOutstation &outstation, unordered_map<int, int> &r_only, unordered_map<int, int> &r_and_w, modbus_t *slave)
{ // reading from modbus and filling the DNP3 database
    UpdateBuilder builder;
    while (!stopFlag)
    {
        for (auto i : r_only)
        {
            uint16_t val[2];
            modbus_read_input_registers(slave, i.first, 1, val);
            builder.Update(Analog(*val), i.second);
        }
        for (auto i : r_and_w)
        {
            uint16_t val[2];
            modbus_read_registers(slave, i.first, 1, val);
            builder.Update(AnalogOutputStatus(*val), i.second);
        }
        outstation.Apply(builder.Build());
        this_thread::sleep_for(std::chrono::seconds(1));
        cout << "data updated ... \n";
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "input modbus server ip\n";
        return 0;
    }
    char *ip = argv[1];                                        // ip of modbus server
    int port = 502;                                            // port of modbus server
    const auto logLevels = levels::NORMAL | levels::ALL_COMMS; // logging for DNP3 server
    DNP3Manager manager(1, ConsoleLogger::Create());
    auto channel = std::shared_ptr<IChannel>(nullptr);
    channel = manager.AddTCPServer("server", logLevels, ServerAcceptMode::CloseExisting, IPEndpoint("0.0.0.0", 20000), PrintingChannelListener::Create()); // TCP server instantiation
    auto myCommandHandler = std::make_shared<MyCommandHandler>();                                                                                          // command handler instantiation
    myCommandHandler->slv = modbus_new_tcp(ip, port);                                                                                                      // modbus client instantiation
    if (myCommandHandler->slv == NULL)
    {
        std::cerr << "Failed to create Modbus context  " << modbus_strerror(errno) << std::endl;
        return -1;
    }
    if (modbus_connect(myCommandHandler->slv) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(myCommandHandler->slv);
        return -1;
    }
    unordered_map<int, int> r_only, r_and_w;           // data mapping from modbus to DNP3 for read only and read and write
    OutstationStackConfig config(ConfigureDatabase()); // DNP# server database
    config.outstation.eventBufferConfig = EventBufferConfig::AllTypes(100);
    config.outstation.params.allowUnsolicited = true;
    config.link.LocalAddr = 10; // DNP3 slave/server address
    config.link.RemoteAddr = 1; // DNP3 master address
    config.link.KeepAliveTimeout = TimeDuration::Max();
    auto outstation = channel->AddOutstation("outstation", myCommandHandler, app, config);
    outstation->Enable(); // enabling the outstation
    csvhandling(r_only, r_and_w, myCommandHandler->mapping);
    thread continuousThread(modbus_read_a, ref(*outstation), ref(r_only), ref(r_and_w), ref(myCommandHandler->slv)); // thread to mandle modbus read
    string exit;
    while (cin >> exit)
    {
        if (exit == "exit")
            break;
        else
            continue;
    }
    stopFlag = TRUE;
    continuousThread.join();
    modbus_close(myCommandHandler->slv);
    modbus_free(myCommandHandler->slv);
}