#ifndef W65C02_ASM_DOCUMENT_H
#define W65C02_ASM_DOCUMENT_H
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

// UI-independent editor buffer. Positions are byte offsets in assembly source.
class AsmDocument {
public:
    explicit AsmDocument(std::string text = {});
    const std::string& text() const { return source; }
    const std::string& savedText() const { return saved; }
    bool dirty() const { return source != saved || !onDisk; }
    bool onDisk = true;
    size_t cursor = 0, anchor = 0;
    size_t topLine = 0, leftColumn = 0;
    void markSaved();
    void reload(std::string text);
    void setCursor(size_t position, bool extend = false);
    std::pair<size_t,size_t> selection() const;
    std::string selectedText() const;
    void insert(const std::string& text);
    void backspace();
    void eraseForward();
    void moveHorizontal(int direction, bool extend, bool word = false);
    void moveVertical(int lines, bool extend);
    void home(bool extend, bool wholeDocument = false);
    void end(bool extend, bool wholeDocument = false);
    void indent(bool outdent);
    void newline();
    void undo();
    void redo();
    size_t lineCount() const;
    size_t lineStart(size_t line) const;
    size_t lineEnd(size_t line) const;
    std::pair<size_t,size_t> location(size_t position) const;
    static std::string normalize(const std::string& text);
private:
    struct State { std::string text; size_t cursor, anchor; };
    std::string source, saved;
    std::vector<State> undoStack, redoStack;
    void checkpoint();
    void replaceSelection(const std::string& text);
};
#endif
