#include "AsmDocument.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace { constexpr size_t MaxSource = 1024 * 1024; }
std::string AsmDocument::normalize(const std::string& text) {
    std::string result;
    for(size_t i=0;i<text.size();++i) {
        if(text[i]=='\r') {
            result+='\n';
            if(i+1<text.size() && text[i+1]=='\n') ++i;
        } else if(text[i]=='\t') result+="    ";
        else result+=text[i];
    }
    if(result.size()>MaxSource) throw std::runtime_error("Source files are limited to 1 MiB");
    return result;
}
AsmDocument::AsmDocument(std::string text) : source(normalize(text)), saved(source) {}
void AsmDocument::markSaved() { saved=source; onDisk=true; }
void AsmDocument::reload(std::string text) {
    source=normalize(text);saved=source;cursor=anchor=topLine=leftColumn=0;
    undoStack.clear();redoStack.clear();onDisk=true;
}
void AsmDocument::setCursor(size_t p,bool extend) {
    cursor=std::min(p,source.size());if(!extend) anchor=cursor;
}
std::pair<size_t,size_t> AsmDocument::selection() const { return std::minmax(cursor,anchor); }
std::string AsmDocument::selectedText() const {
    auto s=selection();return source.substr(s.first,s.second-s.first);
}
void AsmDocument::checkpoint() {
    undoStack.push_back({source,cursor,anchor});redoStack.clear();
    size_t bytes=0;
    for(const auto& s:undoStack) bytes+=s.text.size();
    while(undoStack.size()>1 && (undoStack.size()>100 || bytes>8*1024*1024)) {
        bytes-=undoStack.front().text.size();undoStack.erase(undoStack.begin());
    }
}
void AsmDocument::replaceSelection(const std::string& text) {
    auto s=selection();source.replace(s.first,s.second-s.first,text);
    cursor=anchor=s.first+text.size();
}
void AsmDocument::insert(const std::string& text) {
    auto value=normalize(text);auto s=selection();
    if(source.size()-(s.second-s.first)+value.size()>MaxSource)
        throw std::runtime_error("Source files are limited to 1 MiB");
    if(value.empty() && cursor==anchor) return;
    checkpoint();replaceSelection(value);
}
void AsmDocument::backspace() {
    if(cursor==anchor && !cursor) return;
    checkpoint();if(cursor==anchor) --anchor;replaceSelection("");
}
void AsmDocument::eraseForward() {
    if(cursor==anchor && cursor==source.size()) return;
    checkpoint();if(cursor==anchor) ++anchor;replaceSelection("");
}
void AsmDocument::moveHorizontal(int direction,bool extend,bool word) {
    auto s=selection();
    if(!extend && cursor!=anchor) { setCursor(direction<0?s.first:s.second);return; }
    size_t p=cursor;
    auto isWord=[](char c){return std::isalnum(static_cast<unsigned char>(c)) || c=='_';};
    if(direction<0 && p) {
        --p;
        if(word) {
            while(p && std::isspace(static_cast<unsigned char>(source[p]))) --p;
            while(p && isWord(source[p-1])==isWord(source[p]) && source[p-1]!='\n') --p;
        }
    } else if(direction>0 && p<source.size()) {
        bool kind=isWord(source[p++]);
        if(word) {
            while(p<source.size() && isWord(source[p])==kind && !std::isspace(static_cast<unsigned char>(source[p]))) ++p;
            while(p<source.size() && std::isspace(static_cast<unsigned char>(source[p]))) ++p;
        }
    }
    setCursor(p,extend);
}
size_t AsmDocument::lineCount() const { return 1+std::count(source.begin(),source.end(),'\n'); }
size_t AsmDocument::lineStart(size_t line) const {
    size_t p=0;
    while(line-- && p<source.size()) {
        auto next=source.find('\n',p);if(next==std::string::npos) return source.size();p=next+1;
    }
    return p;
}
size_t AsmDocument::lineEnd(size_t line) const {
    auto p=source.find('\n',lineStart(line));return p==std::string::npos?source.size():p;
}
std::pair<size_t,size_t> AsmDocument::location(size_t position) const {
    position=std::min(position,source.size());
    size_t line=0,start=0;
    for(size_t i=0;i<position;++i) if(source[i]=='\n') {++line;start=i+1;}
    return {line,position-start};
}
void AsmDocument::moveVertical(int lines,bool extend) {
    auto p=location(cursor);
    size_t line=static_cast<size_t>(std::max<long long>(0,std::min<long long>(lineCount()-1,static_cast<long long>(p.first)+lines)));
    setCursor(std::min(lineStart(line)+p.second,lineEnd(line)),extend);
}
void AsmDocument::home(bool extend,bool whole) { setCursor(whole?0:lineStart(location(cursor).first),extend); }
void AsmDocument::end(bool extend,bool whole) { setCursor(whole?source.size():lineEnd(location(cursor).first),extend); }
void AsmDocument::newline() {
    auto start=lineStart(location(cursor).first);size_t p=start;
    while(p<cursor && source[p]==' ') ++p;
    insert("\n"+source.substr(start,p-start));
}
void AsmDocument::indent(bool outdent) {
    if(cursor==anchor && !outdent) { insert("    ");return; }
    auto s=selection();auto first=location(s.first).first;
    auto last=location(s.second).first;
    if(s.second>s.first && s.second==lineStart(last)) --last;
    auto begin=lineStart(first),finish=lineEnd(last);
    std::string block=source.substr(begin,finish-begin),result;
    for(size_t p=0;;) {
        auto end=block.find('\n',p);if(end==std::string::npos) end=block.size();
        size_t cut=0;
        if(outdent) while(cut<4 && p+cut<end && block[p+cut]==' ') ++cut;
        else result+="    ";
        result+=block.substr(p+cut,end-p-cut);
        if(end==block.size()) break;
        result+='\n';p=end+1;
    }
    if(source.size()-(finish-begin)+result.size()>MaxSource) throw std::runtime_error("Source file is too large");
    checkpoint();source.replace(begin,finish-begin,result);anchor=begin;cursor=begin+result.size();
}
void AsmDocument::undo() {
    if(undoStack.empty()) return;
    redoStack.push_back({source,cursor,anchor});auto s=std::move(undoStack.back());undoStack.pop_back();
    source=std::move(s.text);cursor=s.cursor;anchor=s.anchor;
}
void AsmDocument::redo() {
    if(redoStack.empty()) return;
    undoStack.push_back({source,cursor,anchor});auto s=std::move(redoStack.back());redoStack.pop_back();
    source=std::move(s.text);cursor=s.cursor;anchor=s.anchor;
}
