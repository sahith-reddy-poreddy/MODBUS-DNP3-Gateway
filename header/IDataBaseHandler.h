#pragma once

#include<string.h>
#include<iostream>
#include<unordered_map>
#include<opendnp3/outstation/DatabaseConfig.h>
#include<fstream>
#include<vector>

using namespace std;
using namespace opendnp3;

class DataMap{
    public:
    unordered_map<int,int> CR_Output;
    unordered_map<int,int> A_O_I16;
    unordered_map<int,int> A_O_I32;
    unordered_map<int,int> A_O_F32;
    unordered_map<int,int> A_O_D64;

};

class Database{
    public:
    DatabaseConfig database;

};

class Mapping : public DataMap,public Database{
    void ReadFile(string);
    void LineHandle(string);
    vector<string> LineRead(string);
};