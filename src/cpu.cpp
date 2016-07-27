
#include "cpu.h"
#include "opcodes.h"
#include "cases.h"

#define REPORT_ALL_OPCODES              (0)
#define BREAK_AT_INSTRUCTION            (0xFFFFFFFF) // (59216)
#define BREAK_AT_PC_VALUE               0xC6EA // 0xDF14 // 0x8153 // (0x88D2)

namespace nes {

virtual_cpu::virtual_cpu() {}

virtual_cpu::~virtual_cpu() {}

void virtual_cpu::reset()
{
    if (bus)
    {
        interrupt_signal = 0;

        registers.pc = bus->read_cpu_short(RESET_INTERRUPT_VECTOR); // 0xC000;
        registers.sp = RESET_STACK_OFFSET;
        registers.a = 0;
        registers.x = 0;
        registers.y = 0;

        memset(&registers.status_flags, 0, sizeof(cpu_status_flags));
        registers.status_flags.interrupt_disable = 1;
        registers.status_flags.unused = 1;
    }

    cycle_count = 0;
    instruction_count = 0;
}

void virtual_cpu::attach_system_bus(system_bus *input)
{
    bus = input;
}

void virtual_cpu::step()
{
    for (uint32 i = 0; i < CPU_TICK_CYCLE_COUNT; i++)
    {
        handle_interrupt();

        uint16 previous_pc = registers.pc;

        uint8 opcode = bus->read_cpu_byte(registers.pc);

        /*
        // The following output is used when debugging the cpu against logs from nestest.nes.
        // if (instruction_count == BREAK_AT_INSTRUCTION || BREAK_AT_PC_VALUE == registers.pc)
        {
            printf("%04X            %s                             A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%u\n",
                registers.pc, op_name_table[opcode], registers.a, registers.x, registers.y, registers.status_byte, registers.sp, instruction_count);

            // debug_break();
        }
        */

        execute_opcode(opcode);
    
        cycle_count += op_cycle_table[opcode];

        if (is_branch_opcode(opcode))
        {
            if (registers.pc != previous_pc + op_length_table[opcode])
            {
                // a branch or jump was taken, adjust our cycle cost to account for this.
                cycle_count += 1 + ((registers.pc & 0xFF00) != (previous_pc & 0xFF00));
            }
        }
    }
}

void virtual_cpu::fire_interrupt(uint16 input)
{
    if (registers.status_flags.interrupt_disable)
    {
        if (NON_MASKABLE_INTERRUPT_VECTOR == input)
        {
            interrupt_signal = NON_MASKABLE_INTERRUPT_VECTOR;
        }
    }
    else
    {
        interrupt_signal = input;
    }
}

void virtual_cpu::handle_interrupt()
{
    if (interrupt_signal)
    {
        _execute_opcode_int(interrupt_signal, bus, registers);

        interrupt_signal = 0;
        cycle_count += 7;
    }
}

uint16 virtual_cpu::decode_operand(uint8 op)
{
    uint16 operand = 0;

    switch (op_address_mode_table[op])
    {
        case ADDRESS_MODE_INVALID:
        {
            // We do not support a number of 6502 extended opcodes, but we can still skip past most of them.
            // base_msg("Unrecognized addressing mode for opcode %s (0x%x) at cycle %u", op_name_table[op], op, instruction_count);
            // base_post_error(BASE_ERROR_INVALID_RESOURCE);
            break;

        } break;

        // Absolute address mode relies on full 16 bit addresses, while indexed indirect addressing 
        // references the zero page with wrap around. Indirect mode is used only for JMP, and also 
        // requires a page level wrap-around.

        case ADDRESS_MODE_ABSOLUTE: operand = bus->read_cpu_short(registers.pc + 1); break;
        case ADDRESS_MODE_ABSOLUTE_X_INDEXED: operand = bus->read_cpu_short(registers.pc + 1) + registers.x; break;
        case ADDRESS_MODE_ABSOLUTE_Y_INDEXED: operand = bus->read_cpu_short(registers.pc + 1) + registers.y; break;
        case ADDRESS_MODE_ACCUMULATOR: break;
        case ADDRESS_MODE_IMMEDIATE: operand = registers.pc + 1; break; 
        case ADDRESS_MODE_RELATIVE: operand = ((int32) registers.pc + ((int8) bus->read_cpu_byte(registers.pc + 1)) + op_length_table[op]) & 0xFFFF; break;
        case ADDRESS_MODE_ZERO_PAGE: operand = bus->read_cpu_byte(registers.pc + 1); break;
        case ADDRESS_MODE_ZERO_PAGE_X_INDEXED: operand = ((uint16) bus->read_cpu_byte(registers.pc + 1) + registers.x) & 0xFF; break;
        case ADDRESS_MODE_ZERO_PAGE_Y_INDEXED: operand = ((uint16) bus->read_cpu_byte(registers.pc + 1) + registers.y) & 0xFF; break;
        case ADDRESS_MODE_IMPLIED: break; 

        // Indirect addressing suffers from a well documented 6502 bug. If the low byte of the 16 bit address
        // is the last byte in a page, then the high byte will be fetched from the first byte of the *current*
        // page, rather than the next page as one might expect. This affects all three of our indirect modes.

        case ADDRESS_MODE_INDIRECT:
        {
            uint16 jump_target = bus->read_cpu_short(registers.pc + 1);

            if (0xFF == (jump_target & 0xFF))
            {
                operand = bus->read_cpu_byte(jump_target) | ((uint16) bus->read_cpu_byte(jump_target & 0xFF00) << 8);
            }
            else
            {
                operand = bus->read_cpu_short(jump_target);
            }

        } break;

        case ADDRESS_MODE_INDIRECT_PRE_X_INDEXED: 
        {
            uint16 jump_target = ((uint16) bus->read_cpu_byte(registers.pc + 1) + registers.x) & 0xFF;

            if (0xFF == jump_target)
            {
                operand = bus->read_cpu_byte(jump_target) | ((uint16) bus->read_cpu_byte(jump_target & 0xFF00) << 8);
            }
            else
            {
                operand = bus->read_cpu_short(jump_target);
            }
                
        } break;

        case ADDRESS_MODE_INDIRECT_POST_Y_INDEXED: 
        {
            uint16 jump_target = bus->read_cpu_byte(registers.pc + 1);

            if (0xFF == (jump_target & 0xFF))
            {
                operand = bus->read_cpu_byte(jump_target) | ((uint16) bus->read_cpu_byte(jump_target & 0xFF00) << 8);
            }
            else
            {
                operand = bus->read_cpu_short(jump_target);
            }

            operand += registers.y;

        } break;
    }; 

    return operand;
}

bool virtual_cpu::is_branch_opcode(uint8 op)
{
    switch (op)
    {
        OP_BCC: 
        OP_BCS: 
        OP_BEQ: 
        OP_BMI: 
        OP_BNE: 
        OP_BPL: 
        OP_BVC:
        OP_BVS: return true;
    }

    return false;
}

void virtual_cpu::execute_opcode(uint8 op)
{
    uint16 operand_address = decode_operand(op);
    registers.pc += op_length_table[op];
    instruction_count++;

    switch (op)
    {
        OP_ACC_ASL: _execute_opcode_acc_asl(operand_address, bus, registers); break;
        OP_ACC_LSR: _execute_opcode_acc_lsr(operand_address, bus, registers); break;
        OP_ACC_ROL: _execute_opcode_acc_rol(operand_address, bus, registers); break; 
        OP_ACC_ROR: _execute_opcode_acc_ror(operand_address, bus, registers); break;

        OP_NOP: break;

        OP_ADC: _execute_opcode_adc(operand_address, bus, registers); break;
        OP_AND: _execute_opcode_and(operand_address, bus, registers); break;
        OP_ASL: _execute_opcode_asl(operand_address, bus, registers); break;
        OP_BCC: _execute_opcode_bcc(operand_address, bus, registers); break;
        OP_BCS: _execute_opcode_bcs(operand_address, bus, registers); break;
        OP_BEQ: _execute_opcode_beq(operand_address, bus, registers); break;
        OP_BIT: _execute_opcode_bit(operand_address, bus, registers); break;
        OP_BMI: _execute_opcode_bmi(operand_address, bus, registers); break;
        OP_BNE: _execute_opcode_bne(operand_address, bus, registers); break;
        OP_BPL: _execute_opcode_bpl(operand_address, bus, registers); break;
        OP_BRK: _execute_opcode_brk(operand_address, bus, registers); break;
        OP_BVC: _execute_opcode_bvc(operand_address, bus, registers); break;
        OP_BVS: _execute_opcode_bvs(operand_address, bus, registers); break;
        OP_CMP: _execute_opcode_cmp(operand_address, bus, registers); break;
        OP_CPX: _execute_opcode_cpx(operand_address, bus, registers); break;
        OP_CPY: _execute_opcode_cpy(operand_address, bus, registers); break;
        OP_DEC: _execute_opcode_dec(operand_address, bus, registers); break;
        OP_DEX: _execute_opcode_dex(operand_address, bus, registers); break;
        OP_DEY: _execute_opcode_dey(operand_address, bus, registers); break;
        OP_EOR: _execute_opcode_eor(operand_address, bus, registers); break;
        OP_INC: _execute_opcode_inc(operand_address, bus, registers); break;
        OP_INX: _execute_opcode_inx(operand_address, bus, registers); break;
        OP_INY: _execute_opcode_iny(operand_address, bus, registers); break;
        OP_JMP: _execute_opcode_jmp(operand_address, bus, registers); break;
        OP_JSR: _execute_opcode_jsr(operand_address, bus, registers); break;
        OP_LDA: _execute_opcode_lda(operand_address, bus, registers); break;
        OP_LDX: _execute_opcode_ldx(operand_address, bus, registers); break;
        OP_LDY: _execute_opcode_ldy(operand_address, bus, registers); break;
        OP_LSR: _execute_opcode_lsr(operand_address, bus, registers); break;
        OP_ORA: _execute_opcode_ora(operand_address, bus, registers); break;
        OP_PHA: _execute_opcode_pha(operand_address, bus, registers); break;
        OP_PHP: _execute_opcode_php(operand_address, bus, registers); break;
        OP_PLA: _execute_opcode_pla(operand_address, bus, registers); break;
        OP_PLP: _execute_opcode_plp(operand_address, bus, registers); break; 
        OP_ROL: _execute_opcode_rol(operand_address, bus, registers); break;
        OP_ROR: _execute_opcode_ror(operand_address, bus, registers); break;
        OP_RTI: _execute_opcode_rti(operand_address, bus, registers); break;
        OP_RTS: _execute_opcode_rts(operand_address, bus, registers); break;
        OP_SBC: _execute_opcode_sbc(operand_address, bus, registers); break;
        OP_STA: _execute_opcode_sta(operand_address, bus, registers); break;
        OP_STX: _execute_opcode_stx(operand_address, bus, registers); break;
        OP_STY: _execute_opcode_sty(operand_address, bus, registers); break;
        OP_TAX: _execute_opcode_tax(operand_address, bus, registers); break;
        OP_TAY: _execute_opcode_tay(operand_address, bus, registers); break;
        OP_TSX: _execute_opcode_tsx(operand_address, bus, registers); break;
        OP_TXA: _execute_opcode_txa(operand_address, bus, registers); break;
        OP_TXS: _execute_opcode_txs(operand_address, bus, registers); break;
        OP_TYA: _execute_opcode_tya(operand_address, bus, registers); break;

        case 0x18: registers.status_flags.carry = 0; break;
        case 0xD8: registers.status_flags.decimal_mode = 0;  break;
        case 0x58: registers.status_flags.interrupt_disable = 0; break;
        case 0xB8: registers.status_flags.overflow = 0; break;
        case 0x38: registers.status_flags.carry = 1; break;
        case 0xF8: registers.status_flags.decimal_mode = 1; break;
        case 0x78: registers.status_flags.interrupt_disable = 1; break;

        default: 
        {
            // base_msg("Unsupported opcode detected: 0x%x", op);
            // base_post_error(BASE_ERROR_INVALID_RESOURCE);

        } break;
    }
}

} // namespace nes