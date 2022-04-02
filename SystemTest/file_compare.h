#ifndef _65C02_SYSTEM_EMULATION_FILE_COMPARE_H
#define _65C02_SYSTEM_EMULATION_FILE_COMPARE_H

#include <fstream>

std::string compareFiles(const char* file1, const char* file2) {
    std::string string1, string2;
    std::ifstream inputFile1(file1), inputFile2(file2);
    int i{1};
    while(inputFile1.good() and inputFile2.good()) {
        std::getline(inputFile1, string1);
        std::getline(inputFile2, string2);
        if(strcmp(string1.c_str(), string2.c_str()) != 0) return "Line " + std::to_string(i) + " - Expected: " + string1 + ", Actual: " + string2;
        ++i;
    }
    if(inputFile1.seekg(0, std::ios::end).tellg() !=
    inputFile2.seekg(0, std::ios::end).tellg()) return "File sizes unequal";
    return "Equal";
}
#endif //_65C02_SYSTEM_EMULATION_FILE_COMPARE_H
