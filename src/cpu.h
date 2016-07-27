
/*
// Copyright (c) 1998-2008 Joe Bertolami. All Right Reserved.
//
// cpu.h
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Additional Information:
//
//   For more information, visit http://www.bertolami.com.
*/

#ifndef __2A03_CPU_H__
#define __2A03_CPU_H__

#include "base.h"
#include "bus.h"

#define CPU_CLOCK_FREQUENCY                 (1789773)
#define CPU_TICK_CYCLE_COUNT                (113)

#define RESET_STACK_OFFSET                  (0xFD)
#define STACK_BASE_ADDRESS                  (0x100)
#define STATUS_BREAK_MASK                   (0x10)

namespace nes {

using namespace base;      

typedef struct cpu_status_flags
{
    bool carry : 1;
    bool zero : 1;
    bool interrupt_disable : 1;
    bool decimal_mode : 1;
    bool break_command : 1;
    bool unused : 1;
    bool overflow : 1;
    bool negative : 1;

} cpu_status_flags;

typedef struct cpu_register_set
{
    uint16 pc;      // program counter
    uint8 sp;       // stack pointer
    uint8 a;        // accumulator
    uint8 x;        // index - x
    uint8 y;        // index - y

    union 
    {
        uint8 status_byte;
        cpu_status_flags status_flags;
    };

} cpu_register_set;

class virtual_cpu
{
    cpu_register_set registers;
    uint16 interrupt_signal;
    system_bus *bus;
    uint32 cycle_count;
    uint32 instruction_count;

public:

    virtual_cpu();
    ~virtual_cpu();

    void attach_system_bus(system_bus *input);
    void fire_interrupt(uint16 input);
    void reset();
    void step();

private:

    void handle_interrupt();
    void execute_opcode(uint8 op);
    bool is_branch_opcode(uint8 op);
    uint16 decode_operand(uint8 op);
    uint16 fetch_operand(uint8 addr_mode);
};

} // namespace nes

#endif // __2A03_CPU_H__