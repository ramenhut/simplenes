
#include "nes.h"

namespace nes {

famicom::famicom()
{
    frame = 0;
    game = NULL;

    cpu.attach_system_bus(&bus);
    ppu.attach_system_bus(&bus);
    
    bus.attach_cpu(&cpu);
    bus.attach_ppu(&ppu);
}

famicom::~famicom()
{
    eject_rom();
}

status famicom::insert_rom(const char *filename)
{
    eject_rom();
    
    game = new cartridge;

    if (!game)
    {
        return base_post_error(BASE_ERROR_OUTOFMEMORY);
    }

    if (base_failed(load_game_cartridge(filename, game)))
    {
        return base_post_error(BASE_ERROR_EXECUTION_FAILURE);
    }

    bus.reset();
    bus.load_cartridge_into_memory(game);

    cpu.reset();
    ppu.reset();

    frame = 0;

    return BASE_SUCCESS;
}

void famicom::eject_rom()
{
    if (game)
    {
        unload_game_cartridge(game);
        delete game;
        game = NULL;
    }
}

void famicom::read_frame_buffer(void *output_rgb_image)
{
    ppu.read_frame_buffer(output_rgb_image);
}

void famicom::tick()
{
    if (game)
    {
        for (uint32 i = 0; i < PPU_FRAME_SCANLINE_COUNT; i++)
        {
            cpu.step();
            ppu.step();
        }
    }

    frame++;
}

void famicom::attach_controller(uint8 index, controller *keypad)
{
    bus.attach_controller(index, keypad);
}

} // namespace nes