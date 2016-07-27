
/*
// Copyright (c) 1998-2008 Joe Bertolami. All Right Reserved.
//
// cases.h
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

#ifndef __OP_CASES_H__
#define __OP_CASES_H__

#define OP_ACC_ASL case 0x0A
#define OP_ACC_LSR case 0x4A
#define OP_ACC_ROL case 0x2A
#define OP_ACC_ROR case 0x6A

#define OP_ADC case 0x69: case 0x65: case 0x75: case 0x6D: case 0x7D: case 0x79: case 0x61: case 0x71
#define OP_AND case 0x29: case 0x25: case 0x35: case 0x2D: case 0x3D: case 0x39: case 0x21: case 0x31
#define OP_ASL case 0x06: case 0x16: case 0x0E: case 0x1E
#define OP_BCC case 0x90
#define OP_BCS case 0xB0
#define OP_BEQ case 0xF0
#define OP_BIT case 0x24: case 0x2C
#define OP_BMI case 0x30
#define OP_BNE case 0xD0
#define OP_BPL case 0x10
#define OP_BRK case 0x00
#define OP_BVC case 0x50
#define OP_BVS case 0x70
#define OP_CMP case 0xC9: case 0xC5: case 0xD5: case 0xCD: case 0xDD: case 0xD9: case 0xC1: case 0xD1
#define OP_CPX case 0xE0: case 0xE4: case 0xEC
#define OP_CPY case 0xC0: case 0xC4: case 0xCC
#define OP_DEC case 0xC6: case 0xD6: case 0xCE: case 0xDE
#define OP_DEX case 0xCA
#define OP_DEY case 0x88
#define OP_EOR case 0x49: case 0x45: case 0x55: case 0x4D: case 0x5D: case 0x59: case 0x41: case 0x51
#define OP_INC case 0xE6: case 0xF6: case 0xEE: case 0xFE
#define OP_INX case 0xE8
#define OP_INY case 0xC8
#define OP_JMP case 0x4C: case 0x6C
#define OP_JSR case 0x20
#define OP_LDA case 0xA9: case 0xA5: case 0xB5: case 0xAD: case 0xBD: case 0xB9: case 0xA1: case 0xB1
#define OP_LDX case 0xA2: case 0xA6: case 0xB6: case 0xAE: case 0xBE
#define OP_LDY case 0xA0: case 0xA4: case 0xB4: case 0xAC: case 0xBC
#define OP_LSR case 0x46: case 0x56: case 0x4E: case 0x5E
#define OP_NOP case 0xEA
#define OP_ORA case 0x09: case 0x05: case 0x15: case 0x0D: case 0x1D: case 0x19: case 0x01: case 0x11
#define OP_PHA case 0x48
#define OP_PHP case 0x08
#define OP_PLA case 0x68
#define OP_PLP case 0x28
#define OP_ROL case 0x26: case 0x36: case 0x2E: case 0x3E
#define OP_ROR case 0x66: case 0x76: case 0x6E: case 0x7E
#define OP_RTI case 0x40
#define OP_RTS case 0x60
#define OP_SBC case 0xE9: case 0xE5: case 0xF5: case 0xED: case 0xFD: case 0xF9: case 0xE1: case 0xF1
#define OP_STA case 0x85: case 0x95: case 0x8D: case 0x9D: case 0x99: case 0x81: case 0x91
#define OP_STX case 0x86: case 0x96: case 0x8E
#define OP_STY case 0x84: case 0x94: case 0x8C
#define OP_TAX case 0xAA
#define OP_TAY case 0xA8
#define OP_TSX case 0xBA
#define OP_TXA case 0x8A
#define OP_TXS case 0x9A
#define OP_TYA case 0x98

#endif // __OP_CASES_H__