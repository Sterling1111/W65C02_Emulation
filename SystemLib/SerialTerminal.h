#ifndef W65C02_SERIAL_TERMINAL_H
#define W65C02_SERIAL_TERMINAL_H
#include <deque>
#include <string>

// Bounded teletype transcript, independent of SFML and the CPU worker.
class SerialTerminal {
public:
    void append(const std::string& bytes);
    void clear();
    const std::deque<std::string>& lines() const { return history; }
    std::string text() const;
    static std::string input(const std::string& text);
private:
    std::deque<std::string> history{std::string()};
    size_t column{};
};
#endif
