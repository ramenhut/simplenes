
/*
// Copyright (c) 1998-2008 Joe Bertolami. All Right Reserved.
//
// cart.h
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

#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

#include "base.h"

#define PROGRAM_PAGE_SIZE               (0x4000)
#define TILE_PAGE_SIZE                  (0x2000)
#define SAVE_RAM_PAGE_SIZE              (0x2000)

namespace nes {

using namespace base;

#pragma pack( push )
#pragma pack( 2 )

typedef struct rom_header 
{
    char magic[4];

    uint8 prg_page_count;       // 16 KB page size
    uint8 tile_page_count;      // 8 KB page size
    bool mirror_mode : 1;       // 0 - vertical arrangement (horizontal mirror)
                                // 1 - horizontal arrangement (vertical mirror)
    bool sram_avail : 1;        // battery-backed Save RAM
    bool trainer : 1;           // contains a trainer ROM
    bool vram_expansion : 1;    // 0 - two screen nametable set
                                // 1 - four screen nametable set
    uint8 mapper_low : 4;       // low nibble of mapper number
    uint8 reserved_0 : 4;
    uint8 mapper_hi : 4;        // high nibble of mapper number
    uint8 sram_page_count;      // 8 KB page size
    uint8 reserved_1[7];

} rom_header;

#pragma pack(pop)

typedef struct cartridge 
{
    rom_header header;
    uint8 *program_rom;
    uint8 *tile_rom;
    uint8 *save_ram;

} cartridge;

status load_game_cartridge(const char *filename, cartridge *output);

void unload_game_cartridge(cartridge *input);

} // namespace nes

#endif // __CARTRIDGE_H__