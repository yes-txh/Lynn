#ifndef COMMON_CLASSAD_COMPLEMENTION_H
#define COMMON_CLASSAD_COMPLEMENTION_H

#include <string>
#include <classad/classad.h>
#include <classad/classad_distribution.h>

std::string adToString(const ClassAd * ad){
    ClassAdUnParser unparser;
    std::string  classad_text;
        classad_text = "";

    unparser.Unparse(classad_text, ad);

        return classad_text;
}

ClassAd* stringToAd(const std::string &ad){
    ClassAd     *classad;
    ClassAdParser  parser;
    classad = parser.ParseClassAd(ad, true);
    return classad; 
}     
#endif
