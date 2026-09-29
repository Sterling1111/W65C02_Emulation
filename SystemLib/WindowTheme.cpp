#include "WindowTheme.h"
#include <SFML/Window.hpp>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwmapi.h>
#endif

void applyWindowTheme(sf::Window& window, bool active) {
#ifdef _WIN32
    HIGHCONTRASTW contrast{sizeof(HIGHCONTRASTW), 0, nullptr};
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0) &&
        (contrast.dwFlags & HCF_HIGHCONTRASTON)) return;
    const HWND handle = window.getSystemHandle();
    const BOOL dark = TRUE;
    const DWORD rounded = 2; // DWMWCP_ROUND
    const COLORREF caption = RGB(20, 25, 29);
    const COLORREF text = active ? RGB(218, 230, 232) : RGB(132, 151, 158);
    const COLORREF border = active ? RGB(104, 211, 182) : RGB(47, 57, 63);
    // Published DWM attribute IDs; named locally for older MinGW SDK headers.
    constexpr DWORD darkMode = 20, cornerPreference = 33, borderColor = 34;
    constexpr DWORD captionColor = 35, textColor = 36;
    DwmSetWindowAttribute(handle, darkMode, &dark, sizeof(dark));
    DwmSetWindowAttribute(handle, cornerPreference, &rounded, sizeof(rounded));
    DwmSetWindowAttribute(handle, borderColor, &border, sizeof(border));
    DwmSetWindowAttribute(handle, captionColor, &caption, sizeof(caption));
    DwmSetWindowAttribute(handle, textColor, &text, sizeof(text));
    // Keep the native frame: Windows owns snap, resize, system menu, and buttons.
#else
    (void)window;
    (void)active;
#endif
}
