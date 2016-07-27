
/*
// Copyright (c) 1998-2008 Joe Bertolami. All Right Reserved.
//
// bus.h
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

#ifndef __SYSTEM_BUS_H__
#define __SYSTEM_BUS_H__

#include "base.h"
#include "cart.h"
#include "input.h"

#define NON_MASKABLE_INTERRUPT_VECTOR       (0xFFFA)
#define RESET_INTERRUPT_VECTOR              (0xFFFC)
#define BREAK_INTERRUPT_VECTOR              (0xFFFE)

#define SYSTEM_RAM_START                    (0x0000)
#define SYSTEM_PPU_REGISTER_START           (0x2000)
#define SYSTEM_INPUT_REGISTER_START         (0x4000)

#define SYSTEM_RAM_SIZE                     (0x0800)
#define SYSTEM_PPU_REGISTER_SIZE            (0x0008)
#define SYSTEM_INPUT_REGISTER_SIZE          (0x0018)

#define CARTRIDGE_SAVE_RAM_START            (0x6000)
#define CARTRIDGE_PGR_ROM_START             (0x8000)

#define CARTRIDGE_SAVE_RAM_SIZE             (0x2000)
#define CARTRIDGE_PGR_ROM_SIZE              (0x8000)

#define VIDEO_RAM_SIZE                      (0x1000)
#define PALETTE_RAM_SIZE                    (0x0020) 

namespace nes {

using namespace base;

class virtual_cpu;
class virtual_ppu;

class system_bus
{
    uint8 *system_ram;
    uint8 *video_ram;
    uint8 *palette_ram;

    virtual_cpu *cpu;
    virtual_ppu *ppu;

    cartridge *game_cart;
    controller* keypads[2];

public:

    system_bus();
    ~system_bus();

    void load_cartridge_into_memory(cartridge *input);
    void attach_ppu(virtual_ppu *input);
    void attach_cpu(virtual_cpu *input);
    void attach_controller(uint8 index, controller *cont);
    void reset();

    void fire_interrupt(uint16 interrupt_address);

    uint8 read_cpu_byte(uint16 address);
    uint16 read_cpu_short(uint16 address);

    void write_cpu_byte(uint16 address, uint8 input);
    void write_cpu_short(uint16 address, uint16 input);

    uint8 read_ppu_byte(uint16 address);
    uint16 read_ppu_short(uint16 address);

    void write_ppu_byte(uint16 address, uint8 input);
    void write_ppu_short(uint16 address, uint16 input);

    uint32 query_current_scanline();
    rom_header *query_rom_header();
};

} // namespace nes

#endif // __SYSTEM_BUS_H__