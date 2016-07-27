## SimpleNES NES Emulator
---
SimpleNES is a basic NES (or Famicom) emulator that I wrote in early 2008. This emulator faithfully mimics the behavior of the Ricoh 2A03 CPU and 2C02 PPU within the original Nintendo system, and is able to successfully boot and play many different kinds of games. 

Notable features include:
* Semi-cycle-accurate CPU and PPU cycle emulation (but not pixel level)
* Functionally precise emulation of all NES supported 6502 opcodes
* Emulates the well known 6502 indirect addressing bug
* Support for vertical and horizontal scrolling
* Emulation of sprite zero hit and max hits per scanline flags
* 8x8 sprite and background tile rendering
* Properly mirrored palette addresses
* Support for proper screen clipping
* Sprite layering with correct priorities
* iNES ROM file support
* Support for two controllers
* Output display rendering using OpenGL

### More Information
For more information visit my SimpleNES project page at [http://www.bertolami.com](http://localhost:8080/draft6/index.php?engine=portfolio&content=emulation&detail=nes-emulator).
