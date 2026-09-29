#include "SerialTerminal.h"
#include <stdexcept>
void SerialTerminal::clear() { history={std::string()};column=0; }
void SerialTerminal::append(const std::string& bytes) {
    for(unsigned char c:bytes) {
        c&=0x7f;
        if(c=='\r')column=0;
        else if(c=='\n') {history.emplace_back();column=0;}
        else if(c==8 || c==127) {if(column)--column;}
        else if(c>=32) {
            if(column>=100) {history.emplace_back();column=0;}
            auto& line=history.back();
            if(line.size()<=column)line.resize(column+1,' ');
            line[column++]=char(c);
        }
        if(history.size()>2000)history.pop_front();
    }
}
std::string SerialTerminal::text() const {
    std::string result;
    for(size_t i=0;i<history.size();++i) {if(i)result+='\n';result+=history[i];}
    return result;
}
std::string SerialTerminal::input(const std::string& text) {
    std::string result;
    for(size_t i=0;i<text.size();++i) {
        const auto c=static_cast<unsigned char>(text[i]);
        if(c=='\r') {result+='\r';if(i+1<text.size() && text[i+1]=='\n')++i;}
        else if(c=='\n')result+='\r';
        else if(c=='\t')result+=' ';
        else if((c>=32 && c<127)||c==3||c==8||c==27)result+=char(c);
        else throw std::runtime_error("The serial terminal accepts ASCII text.");
    }
    return result;
}
