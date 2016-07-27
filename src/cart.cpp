
#include "cart.h"

namespace nes {

static bool rom_file_has_integrity(const rom_header &header)
{
    if (header.magic[0] != 'N' || header.magic[1] != 'E' ||
        header.magic[2] != 'S' || header.magic[3] != 0x1a)
    {
        return false;
    }

    if (!header.prg_page_count || !header.tile_page_count)
    {
        return false;
    }
    
    if (header.sram_page_count >= 2)
    {
        return false;
    }

    return true;
}

static bool rom_file_has_unsupported_features(const rom_header &header)
{
    return (header.trainer || header.sram_avail || header.vram_expansion);
}

status load_game_cartridge(const char *filename, cartridge *output)
{
    if (BASE_PARAM_CHECK)
    {
        if (!filename || !output)
        {
            return base_post_error(BASE_ERROR_INVALIDARG);
        }
    }

    FILE *rom_file = fopen(filename, "rb");

    if (!rom_file)
    {
        return base_post_error(BASE_ERROR_EXECUTION_FAILURE);
    }

    if (16 != fread(&output->header, 1, 16, rom_file))
    {
        fclose(rom_file);
        return base_post_error(BASE_ERROR_EXECUTION_FAILURE);
    }

    if (!rom_file_has_integrity(output->header))
    {
        fclose(rom_file);
        return base_post_error(BASE_ERROR_EXECUTION_FAILURE);
    }

    if (rom_file_has_unsupported_features(output->header))
    {
        fclose(rom_file);
        return base_post_error(BASE_ERROR_EXECUTION_FAILURE);
    }

    uint32 program_data_size = PROGRAM_PAGE_SIZE * output->header.prg_page_count;
    uint32 tile_data_size = TILE_PAGE_SIZE * output->header.tile_page_count;
    uint32 save_ram_size = SAVE_RAM_PAGE_SIZE * max(1, output->header.sram_page_count);

    output->program_rom = new uint8[program_data_size];
    output->tile_rom = new uint8[tile_data_size];
    output->save_ram = new uint8[save_ram_size];

    if (!output->program_rom || !output->tile_rom || !output->save_ram)
    {
        fclose(rom_file);
        unload_game_cartridge(output);
        return base_post_error(BASE_ERROR_OUTOFMEMORY);
    }

    memset(output->program_rom, 0, program_data_size);
    memset(output->tile_rom, 0, tile_data_size);
    memset(output->save_ram, 0, save_ram_size);

    if (program_data_size != fread(output->program_rom, 1, program_data_size, rom_file) ||
        tile_data_size != fread(output->tile_rom, 1, tile_data_size, rom_file))
    {
        fclose(rom_file);
        unload_game_cartridge(output);
        return base_post_error(BASE_ERROR_OUTOFMEMORY);
    }

    return BASE_SUCCESS;
}

void unload_game_cartridge(cartridge *input)
{
    if (BASE_PARAM_CHECK)
    {
        if (!input)
        {
            base_post_error(BASE_ERROR_INVALIDARG);
            return;
        }
    }

    delete [] input->program_rom;
    delete [] input->tile_rom;
    delete [] input->save_ram;

    input->program_rom = NULL;
    input->tile_rom = NULL;
    input->save_ram = NULL;
}

} // namespace nes