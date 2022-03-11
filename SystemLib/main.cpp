#include <iostream>
#include "System.h"
#include <iostream>

int main() {
    System system{0x0000, 0x3FFF, 0x6000, 0x7FFF, 0x8000, 0xFFFF, .001};
    system.executeProgram("a.out", 47, true, "emulation_load_register.txt");
    return 0;
}

