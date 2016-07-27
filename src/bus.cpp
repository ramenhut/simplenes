
#include "bus.h"
#include "ppu.h"
#include "cpu.h"

namespace nes {

system_bus::system_bus()
{
    game_cart = NULL;
    system_ram = new uint8[SYSTEM_RAM_SIZE];
    video_ram = new uint8[VIDEO_RAM_SIZE];
    palette_ram = new uint8[PALETTE_RAM_SIZE];
    keypads[0] = keypads[1] = NULL;

    if (!system_ram || !video_ram || !palette_ram)
    {
        base_post_error(BASE_ERROR_OUTOFMEMORY);
    }
}

void system_bus::reset()
{
    memset(system_ram, 0, SYSTEM_RAM_SIZE);
    memset(video_ram, 0, VIDEO_RAM_SIZE);
    memset(palette_ram, 0, PALETTE_RAM_SIZE);
}

system_bus::~system_bus()
{
    delete [] system_ram;
    delete [] video_ram;
    delete [] palette_ram;
}

uint32 system_bus::query_current_scanline()
{
    if (ppu)
    {
        return ppu->query_current_scanline();
    }

    return 0;
}

void system_bus::load_cartridge_into_memory(cartridge *input)
{
    if (!ppu || !cpu)
    {
        base_post_error(BASE_ERROR_INVALID_RESOURCE);
    }

    game_cart = input;

    ppu->set_mirror_mode(game_cart->header.mirror_mode);
}

rom_header *system_bus::query_rom_header()
{
    return &game_cart->header;
}

void system_bus::attach_ppu(virtual_ppu *input)
{
    ppu = input;
}

void system_bus::attach_cpu(virtual_cpu *input)
{
    cpu = input;
}

void system_bus::attach_controller(uint8 index, controller *cont)
{
    switch (index)
    {
        case 0: keypads[0] = cont; return;
        case 1: keypads[1] = cont; return;
    }

    base_post_error(BASE_ERROR_INVALIDARG);
}

void system_bus::fire_interrupt(uint16 interrupt_address)
{
    cpu->fire_interrupt(interrupt_address);
}

uint8 system_bus::read_cpu_byte(uint16 address)
{
    if (BASE_PARAM_CHECK)
    {
        if (!game_cart || !ppu)
        {
            base_post_error(BASE_ERROR_INVALID_RESOURCE);
        }
    }
        
    if (address >= 0x8000)
    {
        switch (game_cart->header.prg_page_count)
        {
            case 1: return game_cart->program_rom[(address - 0x8000) % 0x4000];
            case 2: return game_cart->program_rom[address - 0x8000];
            default: base_post_error(BASE_ERROR_INVALIDARG);
        }
    }
    else if (address >= 0x6000)
    {
        return game_cart->save_ram[address - 0x6000];
    }
    else if (address >= 0x4000)
    {
        switch (address)
        {
            case 0x4014:
            {
                // DMA reads are not supported.
                base_post_error(BASE_ERROR_INVALID_RESOURCE);

            } break;

            case 0x4016:
            {
                if (keypads[0])
                {
                    return keypads[0]->read();
                }

            } break;

            case 0x4017:
            {
                if (keypads[1])
                {
                    return keypads[1]->read();
                }

            } break;

            default: break; // unsupported IO
        }
    }
    else if (address >= 0x2000)
    {
        return ppu->read_ppu_register((address - 0x2000) & 0x7);
    }
    else
    {
        return system_ram[address & 0x7FF];
    }

    return 0;
}

uint16 system_bus::read_cpu_short(uint16 address)
{
    uint16 low_byte = read_cpu_byte(address);
    uint16 high_byte = read_cpu_byte(address + 1);
    return (high_byte << 8) | low_byte;
}

void system_bus::write_cpu_byte(uint16 address, uint8 input)
{
    if (BASE_PARAM_CHECK)
    {
        if (!game_cart)
        {
            base_post_error(BASE_ERROR_INVALID_RESOURCE);
        }
    }

    if (address >= 0x8000)
    {
        base_post_error(BASE_ERROR_INVALID_RESOURCE);
    }
    else if (address >= 0x6000)
    {
        game_cart->save_ram[address - 0x6000] = input;
    }
    else if (address >= 0x4000)
    {
        switch (address)
        {
            case 0x4014: 
            {
                uint16 cpu_address = input << 8;
                ppu->write_oam_block(cpu_address);

            } break;

            case 0x4016:
            {
                if (keypads[0])
                {
                    keypads[0]->write(input);
                }

                if (keypads[1])
                {
                    keypads[1]->write(input);
                }

            } break;
            
            default: break; // unsupported IO write.
        }
    }
    else if (address >= 0x2000)
    {
        ppu->write_ppu_register((address - 0x2000) & 0x7, input);
    }
    else
    {
        system_ram[address & 0x7FF] = input;
    }
}

void system_bus::write_cpu_short(uint16 address, uint16 input)
{
    write_cpu_byte(address, input & 0xFF);
    write_cpu_byte(address + 1, input >> 8);
}

uint8 system_bus::read_ppu_byte(uint16 address)
{
    if (BASE_PARAM_CHECK)
    {
        if (!game_cart)
        {
            base_post_error(BASE_ERROR_INVALID_RESOURCE);
        }

        if (address > 0x3FFF)
        {
            base_post_error(BASE_ERROR_INVALIDARG);
        }
    }
        
    if (address >= 0x3F00)
    {
        if (address >= 0x3F10 && (address % 4) == 0)
        {
		    address -= 0x10;
	    }

        return palette_ram[(address - 0x3F00) & 0x1F];
    }
    else if (address >= 0x2000)
    {
        if (game_cart->header.mirror_mode)
        {
            // Vertical mirroring - $2000 and $2400 contain our horizontal tables, 
            // with mirrors at $2800 and $2C00.
            address &= 0x7FF;
        }
        else
        {
            // Horizontal mirroring - $2000 and $2800 contain our vertical tables, 
            // with mirrors at $2400 and $2C00.
            address &= 0xBFF;
        }

        return video_ram[address];
    }
    else
    {
        return game_cart->tile_rom[address];
    }

    return 0;
}

void system_bus::write_ppu_byte(uint16 address, uint8 input)
{
    if (BASE_PARAM_CHECK)
    {
        if (!game_cart)
        {
            base_post_error(BASE_ERROR_INVALID_RESOURCE);
        }
    }

    if (address >= 0x3FFF)
    {
        base_post_error(BASE_ERROR_INVALID_RESOURCE);
    }
    else if (address >= 0x3F00)
    {
        if (address >= 0x3F10 && (address % 4) == 0)
        {
		    address -= 0x10;
	    }

        palette_ram[(address - 0x3F00) & 0x1F] = input;
    }
    else if (address >= 0x2000)
    {
        if (game_cart->header.mirror_mode)
        {
            // Vertical mirroring - $2000 and $2400 contain our horizontal tables, 
            // with mirrors at $2800 and $2C00.
            address &= 0x7FF;
        }
        else
        {
            // Horizontal mirroring - $2000 and $2800 contain our vertical tables, 
            // with mirrors at $2400 and $2C00.
            address &= 0xBFF;
        }

        video_ram[address] = input;
    }
    else
    {
        game_cart->tile_rom[address] = input;
    }
}

} // namespace nes