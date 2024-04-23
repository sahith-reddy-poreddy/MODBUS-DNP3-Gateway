#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <thread>
#include <fstream>
#include <unordered_map>
#include <atomic>



#include <modbus/modbus.h>
#include <opendnp3/ConsoleLogger.h>
#include <opendnp3/DNP3Manager.h>
#include <opendnp3/channel/PrintingChannelListener.h>
#include <opendnp3/logging/LogLevels.h>
#include <opendnp3/outstation/DefaultOutstationApplication.h>
#include <opendnp3/outstation/IUpdateHandler.h>
#include <opendnp3/outstation/SimpleCommandHandler.h>
#include <opendnp3/outstation/UpdateBuilder.h>



#include <IDataBaseHandler.h>
#include <IDataTypeHandling.h>
#include <MyCommandHandler.h>
#include <IDataBaseUpdater.h>


using namespace std;
using namespace opendnp3;

auto app = DefaultOutstationApplication::Create(); // outstation/server application

std::atomic<bool> stopFlag(false); 

void Read_Modbus(modbus_t* slv,DataMap& Map,IOutstation& outstation){
    UpdateDatabase update;
    while(!stopFlag){
        update.Full_Data_Update(slv,Map,outstation);
        this_thread::sleep_for(std::chrono::seconds(1));
        cout << "data updated ... \n";
    }
    return;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "no input given\n";
        return 0;
    }                                           // port of modbus server
    const auto logLevels = levels::NORMAL | levels::ALL_COMMS; // logging for DNP3 server
    DNP3Manager manager(1, ConsoleLogger::Create());
    auto channel = std::shared_ptr<IChannel>(nullptr);
    channel = manager.AddTCPServer("server", logLevels, ServerAcceptMode::CloseExisting, IPEndpoint("0.0.0.0", 20000), PrintingChannelListener::Create()); // TCP server instantiation
    auto myCommandHandler = std::make_shared<MyCommandHandler>();
    if(string(argv[1])=="tcp"){
        char *ip = argv[2];                                        // ip of modbus server
        int port = 502; 
        myCommandHandler->slv = modbus_new_tcp(ip, port);
    }
    else if(string(argv[1])=="rtu"){
        char* addr= argv[2];
        myCommandHandler->slv=modbus_new_rtu(addr,9600,'N',8,1);
    }   
    else {
        cout<<"invalid input"<<endl;
        return 0;
    }                                                                               // command handler instantiation                                                                                                // modbus client instantiation
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
    int slv_id=stoi(argv[3]);
    modbus_set_slave(myCommandHandler->slv,slv_id);
    DatabaseConfig database=myCommandHandler->mapping.ReadFile("mapping_file.txt");

    OutstationStackConfig config(database); // DNP# server database
    config.outstation.eventBufferConfig = EventBufferConfig::AllTypes(100);
    config.outstation.params.allowUnsolicited = true;
    config.link.LocalAddr = 10; // DNP3 slave/server address
    config.link.RemoteAddr = 1; // DNP3 master address
    config.link.KeepAliveTimeout = TimeDuration::Max();
    auto outstation = channel->AddOutstation("outstation", myCommandHandler, app, config);
    outstation->Enable(); // enabling the outstation
    thread continuousThread(Read_Modbus,ref(myCommandHandler->slv),ref(myCommandHandler->mapping), ref(*outstation));
    string exit;
    while (cin >> exit)
    {
        if (exit == "exit")
            break;
        else
            continue;
    }
    stopFlag = TRUE;
    stopFlag = TRUE;
    continuousThread.join();
    modbus_close(myCommandHandler->slv);
    modbus_free(myCommandHandler->slv);
}