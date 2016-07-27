
#include "ppu.h"

namespace nes {

static const ppu_color ppu_palette[] =
{
    {0x66, 0x66, 0x66}, {0x00, 0x2a, 0x88}, {0x14, 0x12, 0xa7}, {0x3b, 0x00, 0xa4}, 
    {0x5c, 0x00, 0x7e}, {0x6e, 0x00, 0x40}, {0x6c, 0x06, 0x00}, {0x56, 0x1d, 0x00}, 
    {0x33, 0x35, 0x00}, {0x0b, 0x48, 0x00}, {0x00, 0x52, 0x00}, {0x00, 0x4f, 0x08}, 
    {0x00, 0x40, 0x4d}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, 
    {0xad, 0xad, 0xad}, {0x15, 0x5f, 0xd9}, {0x42, 0x40, 0xff}, {0x75, 0x27, 0xfe}, 
    {0xa0, 0x1a, 0xcc}, {0xb7, 0x1e, 0x7b}, {0xb5, 0x31, 0x20}, {0x99, 0x4e, 0x00}, 
    {0x6b, 0x6d, 0x00}, {0x38, 0x87, 0x00}, {0x0c, 0x93, 0x00}, {0x00, 0x8f, 0x32}, 
    {0x00, 0x7c, 0x8d}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, 
    {0xff, 0xfe, 0xff}, {0x64, 0xb0, 0xff}, {0x92, 0x90, 0xff}, {0xc6, 0x76, 0xff}, 
    {0xf3, 0x6a, 0xff}, {0xfe, 0x6e, 0xcc}, {0xfe, 0x81, 0x70}, {0xea, 0x9e, 0x22}, 
    {0xbc, 0xbe, 0x00}, {0x88, 0xd8, 0x00}, {0x5c, 0xe4, 0x30}, {0x45, 0xe0, 0x82}, 
    {0x48, 0xcd, 0xde}, {0x4f, 0x4f, 0x4f}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, 
    {0xff, 0xfe, 0xff}, {0xc0, 0xdf, 0xff}, {0xd3, 0xd2, 0xff}, {0xe8, 0xc8, 0xff}, 
    {0xfb, 0xc2, 0xff}, {0xfe, 0xc4, 0xea}, {0xfe, 0xcc, 0xc5}, {0xf7, 0xd8, 0xa5}, 
    {0xe4, 0xe5, 0x94}, {0xcf, 0xef, 0x96}, {0xbd, 0xf4, 0xab}, {0xb3, 0xf3, 0xcc}, 
    {0xb5, 0xeb, 0xf2}, {0xb8, 0xb8, 0xb8}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, 
};

virtual_ppu::virtual_ppu() 
{
    frame_buffer = new uint8[PPU_FRAME_BUFFER_SIZE];
    sprite_attrib_ram = new uint8[OBJECT_ATTRIB_RAM_SIZE];

    if (!frame_buffer || !sprite_attrib_ram)
    {
        base_post_error(BASE_ERROR_OUTOFMEMORY);
        return;
    }

    reset();
}

virtual_ppu::~virtual_ppu() 
{
    delete [] frame_buffer;
    delete [] sprite_attrib_ram;
}

void virtual_ppu::reset()
{
    memset(frame_buffer, 0, PPU_FRAME_BUFFER_SIZE);
    memset(sprite_attrib_ram, 0, OBJECT_ATTRIB_RAM_SIZE);

    current_scan_line = 0;
    frame_count = 0;
   
    control_byte = 0;
    mask_byte = 0;
    status_byte = 0;
    address_latch = 0;

    ppu_scroll_x = 0;
    ppu_scroll_y = 0;
    ppu_oam_addr = 0;
    ppu_vram_addr = 0;
    ppu_read_buffer = 0;
    ppu_byte_cache = 0;
}

void virtual_ppu::attach_system_bus(system_bus *input)
{
    bus = input;
}

void virtual_ppu::set_mirror_mode(bool mode)
{
    mirror_mode = mode;
}

uint32 virtual_ppu::query_current_scanline()
{
    return current_scan_line;
}

uint8 virtual_ppu::read_ppu_register(uint16 address)
{
    uint8 output = 0;

    switch (address)
    {
        case 0x2: // status register
        {
            // Reading the status register will clear the address latch used by 
            // PPUSCROLL and PPUADDR, as well as the vertical blank flag.

            output = status_byte | (ppu_byte_cache & 0x1F);
            status_flags.vblank_flag = 0;
            address_latch = 0;

        } break;

        case 0x4: // OAM_DATA
        {
            output = sprite_attrib_ram[ppu_oam_addr++];

        } break;

        case 0x7: // NAME DATA
        {
            output = bus->read_ppu_byte(ppu_vram_addr);

            if ((ppu_vram_addr % 0x4000) < 0x3F00)
            {
                uint8 temp = ppu_read_buffer;
                ppu_read_buffer = output;
                output = temp;
	        } 
            else 
            {
                ppu_read_buffer = bus->read_ppu_byte(ppu_vram_addr - 0x1000);
            }

            if (!control_flags.vertical_write)
            {
                ppu_vram_addr++;
            }
            else
            {
                ppu_vram_addr += 32;
            }

        } break;

        default: base_post_error(BASE_ERROR_INVALIDARG);
    };
    
    return output;
}

void virtual_ppu::write_ppu_register(uint16 address, uint8 input)
{
    switch (address)
    {
        case 0x0:
        {
            // If the write is activating vblank NMIs, and we are currently
            // in a vblank period, issue an NMI immediately.

            control_byte = input;

            if (status_flags.vblank_flag && (input & 0x80))
            {
                bus->fire_interrupt(NON_MASKABLE_INTERRUPT_VECTOR);
            }

        } break;

        case 0x1:
        {
            mask_byte = input;

        } break;

        case 0x3: // OAM_ADDR
        {
            ppu_oam_addr = input;

        } break;

        case 0x4: // OAM_DATA
        {
            sprite_attrib_ram[ppu_oam_addr++] = input;

        } break;

        case 0x5: // SCROLL
        {
            if (0 == address_latch)
            {
                ppu_scroll_x = input;
            }
            else
            {
                ppu_scroll_y = input;
            }

            address_latch = !address_latch;

        } break;

        case 0x6: // NAME_ADDR
        {
            if (0 == address_latch)
            {
                uint16 input_address = input;
                ppu_vram_addr = (ppu_vram_addr & 0x00FF) | (input_address << 8);
            }
            else
            {
                ppu_vram_addr = (ppu_vram_addr & 0xFF00) | input;
            }

            ppu_vram_addr &= 0x3FFF;
            address_latch = !address_latch;

        } break;

        case 0x7: // NAME_DATA
        {
            bus->write_ppu_byte(ppu_vram_addr, input);

            if (!control_flags.vertical_write)
            {
                ppu_vram_addr++;
            }
            else
            {
                ppu_vram_addr += 32;
            }

        } break;

        default: base_post_error(BASE_ERROR_INVALIDARG);
    };

    ppu_byte_cache = input;
}

void virtual_ppu::write_oam_block(uint16 cpu_address)
{
    // Writes 256 bytes from cpu_address in RAM, to our internal sprite memory bank. 

    for (uint32 i = 0; i < OBJECT_ATTRIB_RAM_SIZE; i++)
    {
        sprite_attrib_ram[ppu_oam_addr++] = bus->read_cpu_byte(cpu_address + i);
    }
}

void virtual_ppu::read_frame_buffer(void *output_rgb_image)
{
    memcpy(output_rgb_image, frame_buffer + 8 * PPU_FRAME_WIDTH * 3, PPU_DISPLAY_BUFFER_SIZE);
}

void virtual_ppu::step()
{
    // Scanline : Description
    //
    // 0        : dummy (Spr0 reset)
    // 1-20     : dummy
    // 21-260   : actual frame
    // 261      : dummy (NMI)

    if (0 == current_scan_line)
    {
        status_flags.sprite_zero_hit = 0;
        status_flags.sprite_overflow_bit = 0;
    }

    if (20 == current_scan_line)
    {           
        // Disable vertical blank flag
        // status_flags.vblank_flag = 0;
    }

    if (current_scan_line > 20 && current_scan_line < 261)
    {
        if (mask_flags.screen_enabled)
        {
            render_background_to_scanline(current_scan_line - 21);
        }

        if (mask_flags.sprites_enabled)
        {
            status_flags.sprite_zero_hit = 1;
            render_sprites_to_scanline(current_scan_line - 21);
        }
    }
     
    if (261 == current_scan_line)
    {
        status_flags.vblank_flag = 1;

        if (control_flags.vblank_enabled)
        {
            bus->fire_interrupt(NON_MASKABLE_INTERRUPT_VECTOR);
        }
    }

    current_scan_line = (current_scan_line + 1) % PPU_FRAME_SCANLINE_COUNT;
}

void virtual_ppu::render_background_to_scanline(uint8 scanline_y)
{
    // this method is responsible for traversing the list of tiles that are covered 
    // by our current frame. for each tile we fetch the pattern byte and attribute 
    // data, and then call render_pattern.

    uint32 start_x = (mask_flags.screen_mask ? 0 : 8);

    for (uint32 scanline_x = start_x; scanline_x < 256;)
    {
        uint32 pixel_offset = (scanline_y * PPU_FRAME_WIDTH + scanline_x) * 3;

        uint32 nametable_x = scanline_x + ppu_scroll_x;
        uint32 nametable_y = scanline_y + ppu_scroll_y;

        uint8 count = 8 - (nametable_x % 8);
        count = min(count, 256 - scanline_x);

        uint8 pattern_index = fetch_nametable_byte(nametable_x >> 3, nametable_y >> 3);
        uint8 attrib_index = fetch_attrib_byte(nametable_x >> 4, nametable_y >> 4);

        render_background_pattern(&frame_buffer[pixel_offset], pattern_index, attrib_index, nametable_x % 8, nametable_y % 8, count);

        scanline_x += count;
    }
}

uint8 virtual_ppu::fetch_nametable_byte(uint16 tile_x, uint16 tile_y)
{
    // We compute this each pass to catch cpu writes to ppu_control.
    uint16 name_table_select = ((control_byte & 0x3) << 0xA);
    uint16 tile_byte_address = 0x2000 | name_table_select;

    if (mirror_mode)
    {
        // Vertical mirroring - $2000 and $2400 contain 
        // our horizontal tables

        if (tile_y >= 30)
        {
            base_post_error(BASE_ERROR_EXECUTION_FAILURE);
        }

        if (tile_x >= 32)
        {
            tile_byte_address ^= 0x0400;
            tile_x -= 32;
        }
    }
    else
    {
        // Horizontal mirroring - $2000 and $2800 contain 
        // our vertical tables

        if (tile_x >= 32)
        {
            base_post_error(BASE_ERROR_EXECUTION_FAILURE);
        }

        if (tile_y >= 30)
        {
            tile_byte_address ^= 0x0800;
            tile_y -= 30;
        }
    }

    tile_byte_address += tile_y * 32 + tile_x;      
    return bus->read_ppu_byte(tile_byte_address);
}

uint8 virtual_ppu::fetch_attrib_byte(uint16 tile_x, uint16 tile_y)
{
    // We compute this each pass to catch cpu writes to ppu_control.
    uint16 name_table_select = ((control_byte & 0x3) << 0xA);
    uint16 tile_byte_address = 0x23C0 | name_table_select;

    // Our location within the 8x8 grid.
    uint16 attrib_block_x = tile_x >> 1;
    uint16 attrib_block_y = tile_y >> 1;

    // Our location within the byte.
    uint8 sub_tile_x = tile_x % 2;
    uint8 sub_tile_y = tile_y % 2;

    if (mirror_mode)
    {
        // Vertical mirroring - $2000 and $2400 contain 
        // our horizontal tables

        if (attrib_block_y >= 8)
        {
            base_post_error(BASE_ERROR_EXECUTION_FAILURE);
        }

        if (attrib_block_x >= 8)
        {
            tile_byte_address ^= 0x0400;
            attrib_block_x -= 8;
        }
    }
    else
    {
        // Horizontal mirroring - $2000 and $2800 contain 
        // our vertical tables

        if (attrib_block_x >= 8)
        {
            base_post_error(BASE_ERROR_EXECUTION_FAILURE);
        }

        if (attrib_block_y >= 8)
        {
            tile_byte_address ^= 0x0800;
            attrib_block_y -= 8;
        }
    }

    tile_byte_address += attrib_block_y * 8 + attrib_block_x;      
    uint8 attrib_byte = bus->read_ppu_byte(tile_byte_address);
    attrib_byte = (attrib_byte >> (4 * sub_tile_y + 2 * sub_tile_x)) & 0x3;

    return attrib_byte;
}

void virtual_ppu::render_background_pattern(uint8 *dest, uint8 pattern_index, uint8 palette_index, uint8 internal_x, uint8 internal_y, uint8 count)
{
    // check our status bit to see which bank contains our background patterns
    // load the high and low pattern bytes based on internal_y
    // shift in based on internal_x and then render out count pixels to dest.

    uint16 background_pattern_address = (control_flags.screen_pattern_table_addr ? 0x1000 : 0x0000);
    uint16 low_pattern_byte_address = background_pattern_address + pattern_index * 16 + internal_y;
    uint16 high_pattern_byte_address = low_pattern_byte_address + 8; 

    uint8 low_pattern_byte = bus->read_ppu_byte(low_pattern_byte_address);
    uint8 high_pattern_byte = bus->read_ppu_byte(high_pattern_byte_address);

    low_pattern_byte <<= internal_x;
    high_pattern_byte <<= internal_x;

    // prepare our palette base address.
    uint16 palette_base_address = 0;

    switch (palette_index)
    {
        case 0x0: palette_base_address = 0x3F01; break;
        case 0x1: palette_base_address = 0x3F05; break;
        case 0x2: palette_base_address = 0x3F09; break;
        case 0x3: palette_base_address = 0x3F0D; break;
    };

    render_background_pattern_line(dest, low_pattern_byte, high_pattern_byte, palette_base_address, count);    
}

void virtual_ppu::render_background_pattern_line(uint8 *dest, uint8 low_byte, uint8 high_byte, uint16 palette_base_address, uint8 count)
{
    uint8 color_indices[4] = {0};
    uint16 backdrop_address = 0x3F00;

    if (ppu_vram_addr >= 0x3F00 && ppu_vram_addr <= 0x3FFF)
    {
        backdrop_address = ppu_vram_addr;
    }

    color_indices[0] = bus->read_ppu_byte(backdrop_address);
    color_indices[1] = bus->read_ppu_byte(palette_base_address + 0);
    color_indices[2] = bus->read_ppu_byte(palette_base_address + 1);
    color_indices[3] = bus->read_ppu_byte(palette_base_address + 2);
    
    for (uint8 i = 0; i < count; i++)
    {
        uint8 pattern_byte = ((low_byte & 0x80) >> 7) | ((high_byte & 0x80) >> 6);
        render_pixel(dest + i * 3, pattern_byte, color_indices);

        low_byte <<= 1;
        high_byte <<= 1;
    }
}

ppu_sprite_desc *virtual_ppu::fetch_sprite_desc(uint8 index)
{
    ppu_sprite_desc *sprite_list = (ppu_sprite_desc *) sprite_attrib_ram;
    uint8 test = sizeof(ppu_sprite_desc);
    return &sprite_list[index];
}

void virtual_ppu::render_sprites_to_scanline(uint8 scanline_y)
{
    // Render the sprites in reverse order (painter's algorithm). This won't be 
    // able to support all games, but it works fine for the titles we care about.
    ppu_sprite_desc *sprite_indices[8] = {0};

    // First we find the set of sprites that collide with this scanline.
    uint8 sprite_count = gather_sprite_hit_list(scanline_y, sprite_indices);

    if (9 == sprite_count)
    {
        status_flags.sprite_overflow_bit = 1;
    }

    while (sprite_count)
    {
        ppu_sprite_desc *desc = sprite_indices[sprite_count - 1];
        render_one_sprite_to_scanline(desc, scanline_y);
        sprite_count--;
    }
}

uint8 virtual_ppu::gather_sprite_hit_list(uint8 scanline_y, ppu_sprite_desc **sprite_indices)
{
    uint8 sprite_count = 0;
    
    for (int8 sprite = 0; sprite < 64; sprite++)
    {
        // Render sprite into our scanline if it hits it.
        ppu_sprite_desc *desc = fetch_sprite_desc(sprite);

        if (desc->sprite_y >= 240 || desc->sprite_y > scanline_y || (desc->sprite_y + 7) < scanline_y)
        {
            // Sprite does not intersect our current scanline.
            continue;
        }

        if (desc->sprite_x >= 256)
        {
            continue;
        }

        if (sprite_count < 8)
        {
            // Add to our list
            sprite_indices[sprite_count] = desc;
            sprite_count++;
        }

        if (9 == sprite_count)
        {
            break;
        }
    }

    return sprite_count;
}

void virtual_ppu::render_one_sprite_to_scanline(ppu_sprite_desc *desc, uint8 scanline_y)
{
    uint16 internal_y = scanline_y - desc->sprite_y;
    uint32 pixel_offset = (scanline_y * PPU_FRAME_WIDTH + desc->sprite_x) * 3;
    uint8 count = min(8, 256 - desc->sprite_x);
        
    if (!(desc->attributes & 0x20))
    {
        status_flags.sprite_zero_hit = 1;

        // We only render the sprite if it's in front of the background.
        render_sprite_pattern(&frame_buffer[pixel_offset], desc->tile_index, desc->attributes, internal_y, count);
    }
}

void virtual_ppu::render_sprite_pattern(uint8 *dest, uint8 pattern_index, uint8 attributes, uint8 internal_y, uint8 count)
{
    // check our status bit to see which bank contains our background patterns
    // load the high and low pattern bytes based on internal_y
    // shift in based on internal_x and then render out count pixels to dest.

    if (attributes & 0x80)
    {
        internal_y = 7 - internal_y;
    }

    uint8 palette_index = attributes & 0x3;
    uint16 sprite_pattern_address = (control_flags.sprite_pattern_table_addr ? 0x1000 : 0x0000);
    uint16 low_pattern_byte_address = sprite_pattern_address + pattern_index * 16 + internal_y;
    uint16 high_pattern_byte_address = low_pattern_byte_address + 8; 

    uint8 low_pattern_byte = bus->read_ppu_byte(low_pattern_byte_address);
    uint8 high_pattern_byte = bus->read_ppu_byte(high_pattern_byte_address);

    // prepare our palette base address.
    uint16 palette_base_address = 0;

    switch (palette_index)
    {
        case 0x0: palette_base_address = 0x3F11; break;
        case 0x1: palette_base_address = 0x3F15; break;
        case 0x2: palette_base_address = 0x3F19; break;
        case 0x3: palette_base_address = 0x3F1D; break;
    };

    render_sprite_pattern_line(dest, low_pattern_byte, high_pattern_byte, attributes, palette_base_address, count);   
}

void virtual_ppu::render_sprite_pattern_line(uint8 *dest, uint8 low_byte, uint8 high_byte, uint8 attributes, uint16 palette_base_address, uint8 count)
{
    uint8 color_indices[4] = {0};
    uint16 backdrop_address = 0x3F00;

    color_indices[0] = bus->read_ppu_byte(backdrop_address);
    color_indices[1] = bus->read_ppu_byte(palette_base_address + 0);
    color_indices[2] = bus->read_ppu_byte(palette_base_address + 1);
    color_indices[3] = bus->read_ppu_byte(palette_base_address + 2);
    
    if (attributes & 0x40)
    {
        for (uint8 i = 0; i < count; i++)
        {
            uint8 pattern_byte = ((low_byte & 0x1) | ((high_byte & 0x1) << 1));
            render_pixel(dest + i * 3, pattern_byte, color_indices, true);

            low_byte >>= 1;
            high_byte >>= 1;
        }
    }
    else
    {
        for (uint8 i = 0; i < count; i++)
        {
            uint8 pattern_byte = ((low_byte & 0x80) >> 7) | ((high_byte & 0x80) >> 6);
            render_pixel(dest + i * 3, pattern_byte, color_indices, true);

            low_byte <<= 1;
            high_byte <<= 1;
        }
    }
}

void virtual_ppu::render_pixel(uint8 *dest, uint8 pattern, uint8 *color_indices, bool zero_is_transparent)
{
    uint8 color_index = color_indices[pattern];

    if (0 == pattern && zero_is_transparent)
    {
        return;
    }

    dest[0] = ppu_palette[color_index].red;
    dest[1] = ppu_palette[color_index].green;
    dest[2] = ppu_palette[color_index].blue;
}

} // namespace nes