#include "benchmark/benchmark.h"
#include "System.h"

class _6502LoadRegisterBenchmarks : public benchmark::Fixture {
public:
    const static benchmark::TimeUnit TimeUnit = benchmark::TimeUnit::kMicrosecond;
    System system{0x0000, 0xFFFF, -1, -1, -1, -1, .1};
    W65C02& cpu = system.cpu;
    RAM& ram = system.ram;
    _6502LoadRegisterBenchmarks() {/* Iterations(3);*/}
    void SetUp(const ::benchmark::State& state) {
        cpu.reset();
        cpu.PC = 0xFFFC;
        cpu.SP = 0xFE;
    }
    void TearDown(const ::benchmark::State& state) {}

    byte BenchmarkLoadRegisterImmediate(byte, byte W65C02::*);
    byte BenchmarkLoadRegisterZeroPage(byte, byte W65C02::*);
};

byte _6502LoadRegisterBenchmarks::BenchmarkLoadRegisterImmediate(byte opcode, byte W65C02::* Register) {
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = 0x42;
    cpu.execute();
    cpu.PC = 0xFFFC;
    return cpu.*Register;
}

byte _6502LoadRegisterBenchmarks::BenchmarkLoadRegisterZeroPage(byte opcode, byte W65C02::* Register) {
    ram[0xFFFC] = opcode;
    ram[0xFFFD] = 0x42;
    ram[0x0042] = 0x37;
    cpu.execute();
    cpu.PC = 0xFFFC;
    return cpu.*Register;
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDAImmediate)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(W65C02::INS_LDA_IM, &W65C02::A));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDXImmediate)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(W65C02::INS_LDX_IM, &W65C02::X));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDYImmediate)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(W65C02::INS_LDY_IM, &W65C02::Y));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDAZeroPage)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterZeroPage(W65C02::INS_LDA_ZP, &W65C02::A));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDXZeroPage)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterZeroPage(W65C02::INS_LDX_ZP, &W65C02::X));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDYZeroPage)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterZeroPage(W65C02::INS_LDY_ZP, &W65C02::Y));
}

BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDAImmediate)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDXImmediate)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDYImmediate)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDAZeroPage)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDXZeroPage)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDYZeroPage)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();

