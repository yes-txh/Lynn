#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "recordio_jna.h"

using namespace std;

int main(int argc, char** argv) {

    if(argc != 2){
        cout << "Usage :"<<argv[0] << " compression_codec\n";
        return -1;
    }

    char test_data[10][100];
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 100; ++j){
            test_data[i][j] = (char)((i*j)/128);
        }
    }
    int num = atoi(argv[1]);

    char name[20];
    snprintf(name,20,"%s%d","/tmp/bryanxurio",num);
    cout << "OpenRecordWriter " << name << endl;
    RecordWriter *recordWriter = OpenRecordWriter(name,0,num);
    if(recordWriter == NULL){
        cerr << "open error \n";
        return -2;
    }
    for (int i = 0; i < 10; ++i) {
        WriteRecord(recordWriter,test_data[i],100);
    }
    cout << "writeRecord ok \n";
    FlushRecordWriter(recordWriter);
    CloseRecordWriter(recordWriter);
    cout << "CloseRecordWriter ok \n";

    RecordReader *recordReader = OpenRecordReader(name,0);
    if(recordReader == NULL){
        cerr << "open error \n";
        return -3;
    }
    int index = 0;
    const char* data;
    int len =0;
    while ((ReadRecord(recordReader,&data,&len)) != false) {
        cout << "read a Record\n" ;
        if (len != 100){
            cout << "length not match\n";
        }
        for (int i = 0; i < 100; ++i) {
            if(data[i] != test_data[index][i]){
                cout<<	index << "nd record's " << i << "nd byte mismatched.";
            }
        }
        cout << index << "nd record match.\n";
        ++index;
    }
    CloseRecordReader(recordReader);
    cout << "CloseRecordReader ok \n";

    return 0;
}

