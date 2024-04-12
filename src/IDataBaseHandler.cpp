#include<IDataBaseHandler.h>

void Mapping::ReadFile(string file_addr){
    ifstream fout;
    fout.open(file_addr);
    string cn;
    getline(fout,cn);
    uint16_t cnt=stoi(cn);
    DatabaseConfig data(cnt);
    this->database=data;
    while(getline(fout,cn)){
        this->LineHandle(cn);
    }
}
void Mapping::LineHandle(string line){
    vector<string> all_c=LineRead(line);
    if(all_c[0]=="AI_16"){
        
    }
}

vector<string> Mapping::LineRead(string line){

}