#include "System.h"

System::System(sdword ramMin, sdword ramMax, sdword regMin, sdword regMax, sdword romMin, sdword romMax, double Mhz) :
        bus{ram,ramMin,ramMax,registers, regMin, regMax, eeprom,romMin,romMax},
        portBus{lcd}{
    if(ramMax<0x5000 && romMin>0x5003 && regMin>0x5003)bus.connectSerial(acia);
    cpu.connectBus(&bus);
    cpu.cycles.setTickCallback([this] {
        lcd.tick(cpu.cycles.getFrequencyHz());
        bus.tick(cpu.cycles.getFrequencyHz());
    });
    cpu.setCycleDuration(Mhz);
    registers.connectPortBus(&portBus);
}

void System::executeProgram(const std::string& programObjFile, uint64_t instructionsToExecute, bool logging, const std::string& outFile) {
    cpu.stop();
    eeprom.loadProgram(programObjFile);
    registers.reset();
    acia.reset();
    cpu.reset(eeprom[0xFFFC - 0x8000] | eeprom[0xFFFD - 0x8000] << 8);
    bus.log = logging;
    if(!bus.openProgramOutFile(outFile)) { bus.log = false; }
    cpu.execute(instructionsToExecute);
}

void System::loadProgram(const std::string &programObjFile) {
    cpu.stop();
    eeprom.loadProgram(programObjFile);
    cpu.reset(0);
    registers.reset();
    acia.reset();
    bus.log = false;
}


System::~System() {
    // Stop bus accesses before RAM, ROM, and peripherals are destroyed.
    cpu.stop();
}

void System::reset(bool run) {
    cpu.stop();
    {
        std::lock_guard<std::mutex> lock(cpu.stateMutex());
        registers.reset();
        acia.reset();
        firstReset = true;
    }
    if (run) cpu.start();
    cpu.reset(eeprom[0x7FFC] | eeprom[0x7FFD] << 8);
}
