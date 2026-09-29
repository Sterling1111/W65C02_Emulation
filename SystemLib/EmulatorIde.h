#ifndef W65C02_EMULATOR_IDE_H
#define W65C02_EMULATOR_IDE_H
#include "System.h"
#include "AsmDocument.h"
#include "IdeWorkspace.h"
#include "BreadboardView.h"
#include "SerialTerminal.h"
#include <SFML/Graphics.hpp>
#include <future>
#include <map>
#include <memory>
#include <set>

class EmulatorIde {
public:
    EmulatorIde(System& system, std::filesystem::path programs,
                std::filesystem::path assembler, std::filesystem::path builds);
    int run();
private:
    enum class Tab { Editor, Board, Terminal };
    enum class Dialog { Closed, NewProgram, Close, Reload, GoToLine };
    System& system;
    IdeWorkspace workspace;
    std::filesystem::path assembler, builds;
    sf::RenderWindow window;
    sf::Font uiFont, codeFont;
    std::unique_ptr<BreadboardView> board;
    BreadboardView::Snapshot snapshot;
    std::vector<std::string> names;
    std::map<std::string,AsmDocument> documents;
    std::string selected, loadedName, status = "Select a program, edit, then Build & Run.";
    Tab tab = Tab::Editor;
    Dialog dialog = Dialog::Closed;
    std::string dialogText, dialogError, findText;
    bool findOpen = false, dragging = false, loaded = false, paused = false;
    bool keyIrq = false, keyNmi = false, keyReset = false, irqInput = false, nmiInput = false;
    BreadboardView::Button mouseButton = BreadboardView::Button::Released;
    size_t listTop = 0, consoleTop = 0;
    std::vector<std::string> console;
    std::future<AsmBuildResult> buildJob;
    bool building = false, runAfterBuild = false, closingAfterBuild = false;
    bool debugAfterBuild = false;
    bool debugKeyHeld[4]{};
    std::map<std::string, std::set<size_t>> breakpointLines;
    std::vector<word> activeBreakpoints;
    bool breakpointKeyHeld = false;
    std::string loadedSource;
    std::map<uint16_t, size_t> loadedLines;
    std::map<size_t, std::vector<uint16_t>> loadedBreakpointAddresses;
    std::string buildName, buildSource, errorFile;
    int errorLine = 0;
    sf::Clock caretClock;
    SerialTerminal terminal;
    size_t terminalScroll=0;
    int basicStartup=0;
    std::string bootOutput;
    void bootFirmware(bool basic);
    void pollSerial();
    void sendSerial(const std::string& text);
    void drawTerminal();

    AsmDocument* document();
    void selectProgram(const std::string& name);
    void refreshPrograms();
    void saveCurrent();
    void build(bool run, bool debug = false);
    void finishBuild();
    void loadAndRun(const AsmBuildResult& result);
    void setConsole(const std::string& output);
    void keepCursorVisible();
    void findNext();
    void closeRequested();
    void acceptDialog();
    void setTab(Tab next);
    void resetBoard(bool stayPaused = false);
    void pauseOrContinue();
    void stepInstruction();
    void followInstruction();
    void toggleBreakpoint(size_t line);
    void syncBreakpoints();
    void checkBreakpoint();
    bool breakpointBound(size_t line);
    size_t nextSourceLine();
    std::string registerText();
    std::string executionText();
    void releaseInputs();
    void updateInputs();
    void changeClock();
    void handle(const sf::Event& event);
    void editorKey(const sf::Event::KeyEvent& key);
    void mouseDown(sf::Vector2f point);
    void positionCursor(sf::Vector2f point,bool extend);
    void draw();
    void drawEditor();
    void drawBoard();
    void drawDialog();
};
#endif
