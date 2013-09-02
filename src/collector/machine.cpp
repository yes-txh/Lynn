#include <classad/classad.h>
#include <classad/classad_distribution.h>
#include "collector/machine.h"

int32_t Machine::ParseAttr(const string& machine_ad){
    ClassAdParser parser;
    ClassAd *classad = parser.ParseClassAd(machine_ad, true);
    if(!classad->EvaluateAttrString("IP", m_ip)){
        return 1;
    } else if (!classad->EvaluateAttrInt("Port", m_port)) {
        return 2;
    }  
    return 0;
}

string Machine::GetEndpoint(){
    return "";
}
