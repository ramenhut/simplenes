
/*
// Copyright (c) 1998-2008 Joe Bertolami. All Right Reserved.
//
// ppu.h
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
 
#ifndef __2C02_PPU_H__
#define __2C02_PPU_H__

#include "base.h"
#include "bus.h"

#define PPU_FRAME_WIDTH                     (256)
#define PPU_FRAME_HEIGHT                    (240)
#define PPU_DISPLAY_WIDTH                   (PPU_FRAME_WIDTH)
#define PPU_DISPLAY_HEIGHT                  (PPU_FRAME_HEIGHT - 16)

#define PPU_FRAME_BUFFER_SIZE               (PPU_FRAME_WIDTH * PPU_FRAME_HEIGHT * 3)
#define PPU_DISPLAY_BUFFER_SIZE             (PPU_DISPLAY_WIDTH * PPU_DISPLAY_HEIGHT * 3)
#define OBJECT_ATTRIB_RAM_SIZE              (0x100)

#define PPU_CYCLES_PER_SCANLINE             (340)
#define PPU_VBLANK_BEGIN_CYCLE              (260 * PPU_CYCLES_PER_SCANLINE)
#define PPU_RENDER_BEGIN_CYCLE              (20 * PPU_CYCLES_PER_SCANLINE)
#define PPU_FRAME_SCANLINE_COUNT            (262)
#define PPU_HORIZ_MIRROR_MASK               (0x3FFF)

namespace nes {

using namespace base;    

typedef struct ppu_color
{
    uint8 red;
    uint8 green;
    uint8 blue;

} ppu_color;

typedef struct ppu_sprite_desc
{
    uint8 sprite_y;
    uint8 tile_index;
    uint8 attributes;
    uint8 sprite_x;

} ppu_sprite_desc;

typedef struct ppu_control_flags
{
    uint8 name_table_address : 2;
    uint8 vertical_write : 1;
    uint8 sprite_pattern_table_addr : 1;
    uint8 screen_pattern_table_addr : 1;
    uint8 sprite_size : 1;
    uint8 unused : 1;
    uint8 vblank_enabled : 1;

} ppu_control_flags;

typedef struct ppu_mask_flags
{
    uint8 unused : 1;
    uint8 screen_mask : 1;
    uint8 sprite_mask : 1;
    uint8 screen_enabled : 1;
    uint8 sprites_enabled : 1;
    uint8 screen_red_tint : 1;
    uint8 screen_green_tint : 1;
    uint8 screen_blue_tint : 1;

} ppu_mask_flags;

typedef struct ppu_status_flags
{
    uint8 unused : 5;
    uint8 sprite_overflow_bit : 1;
    uint8 sprite_zero_hit : 1;
    uint8 vblank_flag : 1;

} ppu_status_flags;

class virtual_ppu
{
    system_bus *bus;

    uint8 *frame_buffer;
    uint32 frame_count;
    uint32 current_scan_line;
    uint8 *sprite_attrib_ram;

    union 
    {
        uint8 control_byte;
        ppu_control_flags control_flags;
    };

    union
    {
        uint8 mask_byte;
        ppu_mask_flags mask_flags;
    };

    union
    {
        uint8 status_byte;
        ppu_status_flags status_flags;
    };

    uint8 ppu_scroll_x;
    uint8 ppu_scroll_y;
    uint8 ppu_oam_addr;
    uint8 ppu_read_buffer;
    uint16 ppu_vram_addr;
    uint8 ppu_byte_cache;

    bool address_latch;
    bool mirror_mode;

public:

    virtual_ppu();
    ~virtual_ppu();

    void reset();
    void step();

    void attach_system_bus(system_bus *input);
    void set_mirror_mode(bool mode);

    uint8 read_ppu_register(uint16 address);
    void write_ppu_register(uint16 address, uint8 input);
    void write_oam_block(uint16 cpu_address);

    void read_frame_buffer(void *output_rgb_image);

    uint32 query_current_scanline();
    void print_current_name_table();
    
private:

    uint8 fetch_nametable_byte(uint16 tile_x, uint16 tile_y);
    uint8 fetch_attrib_byte(uint16 tile_x, uint16 tile_y);
    ppu_sprite_desc *fetch_sprite_desc(uint8 index);
    uint8 gather_sprite_hit_list(uint8 scanline_y, ppu_sprite_desc **sprite_indices);

    void render_background_to_scanline(uint8 scanline_y);
    void render_background_pattern(uint8 *dest, uint8 pattern_index, uint8 palette_index, uint8 internal_x, uint8 internal_y, uint8 count);
    void render_background_pattern_line(uint8 *dest, uint8 low_byte, uint8 high_byte, uint16 palette_base_address, uint8 count);

    void render_sprites_to_scanline(uint8 scanline_y);
    void render_one_sprite_to_scanline(ppu_sprite_desc *desc, uint8 scanline_y);
    void render_sprite_pattern(uint8 *dest, uint8 pattern_index, uint8 attributes, uint8 internal_y, uint8 count);
    void render_sprite_pattern_line(uint8 *dest, uint8 low_byte, uint8 high_byte, uint8 attributes, uint16 palette_base_address, uint8 count);

    void render_pixel(uint8 *dest, uint8 pattern, uint8 *color_indices, bool zero_is_transparent=false);
};

} // namespace nes

#endif // __2C02_PPU_H__