#include <stdio.h>
#include <zookeeper/zookeeper.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <classad/classad.h>
#include <classad/classad_distribution.h>

using std::string;

int32_t main(int argc, char* argv[]){
    string         classad_string = "[a = 1; b = \"Cardini\"]";
    ClassAd        *classad;
    ClassAdParser  parser;
    classad = parser.ParseClassAd(classad_string, true);
    int32_t i;
    if(!classad->EvaluateAttrInt("a",i)) { 
         printf("error\n");
    } 
    printf("%d\n",i);
 
    string printed_classad;
   
    ClassAdXMLUnParser xml_unparser;
    xml_unparser.SetCompactSpacing(false);
    xml_unparser.Unparse(printed_classad, classad);
    printf("%s\n", printed_classad.c_str());

    printed_classad = "";
    ClassAdUnParser unparser;
    unparser.Unparse(printed_classad, classad);
    printf("%s\n", printed_classad.c_str()); 

    return 0;
}
