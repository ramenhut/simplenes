
#include "opcodes.h"
#include "bus.h"
#include "cpu.h"

#define PUSH_STACK_BYTE(input) \
    do { \
        bus->write_cpu_byte(STACK_BASE_ADDRESS + registers.sp, (input)); \
        registers.sp--; \
    } while (0)

#define POP_STACK_BYTE(output) \
    do { \
        registers.sp++; \
        (output) = bus->read_cpu_byte(STACK_BASE_ADDRESS + registers.sp); \
    } while (0)

#define PUSH_STACK_SHORT(input) \
    do { \
        bus->write_cpu_byte(STACK_BASE_ADDRESS + registers.sp, ((input) >> 8)); \
        bus->write_cpu_byte(STACK_BASE_ADDRESS + registers.sp - 1, ((input) & 0xFF)); \
        registers.sp -= 2; \
    } while (0)

#define POP_STACK_SHORT(output) \
    do { \
        registers.sp += 2; \
        (output) = (bus->read_cpu_byte(STACK_BASE_ADDRESS + registers.sp) << 8) | \
                    bus->read_cpu_byte(STACK_BASE_ADDRESS + registers.sp - 1); \
    } while(0)

#define SAME_SIGN(a, b) (!((a & 0x80) ^ (b & 0x80)))

#define SET_CARRY(reg) registers.status_flags.carry = !!(reg & 0x80)
#define SET_NEGATIVE(reg) registers.status_flags.negative = !!((reg) & 0x80)
#define SET_ZERO(reg) registers.status_flags.zero = !((reg) & 0xFF)
#define SET_NEGATIVE_AND_ZERO(reg) SET_NEGATIVE(reg); SET_ZERO(reg)
#define LOAD_REG(reg) reg = bus->read_cpu_byte(operand_address)
#define STORE_REG(reg) bus->write_cpu_byte(operand_address, reg)

namespace nes {

void _execute_opcode_adc(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint16 result = (uint16) registers.a + operand + registers.status_flags.carry;
    registers.status_flags.carry = !!((result) & 0xFF00);

    SET_NEGATIVE_AND_ZERO(result);

    // If our inputs have the same sign, but our output does not => overflow.
    registers.status_flags.overflow = SAME_SIGN(registers.a, operand) && !SAME_SIGN(operand, result);
    registers.a = result & 0xFF;
}

void _execute_opcode_and(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = registers.a & operand;

    SET_NEGATIVE_AND_ZERO(result);

    registers.a = result;
}

void _execute_opcode_acc_asl(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    // operand has been set to the accumulator.
    uint8 result = registers.a << 1;

    SET_NEGATIVE_AND_ZERO(result);
    SET_CARRY(registers.a);

    registers.a = result;
}

void _execute_opcode_asl(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = operand << 1;

    SET_NEGATIVE_AND_ZERO(result);
    SET_CARRY(operand);

    bus->write_cpu_byte(operand_address, result);
}

void _execute_opcode_cmp(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint16 result = (uint16) registers.a - operand;
    registers.status_flags.carry = registers.a >= operand;

    SET_NEGATIVE_AND_ZERO(result);
}

void _execute_opcode_cpx(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint16 result = (uint16) registers.x - operand;
    registers.status_flags.carry = registers.x >= operand;

    SET_NEGATIVE_AND_ZERO(result);
}

void _execute_opcode_cpy(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint16 result = (uint16) registers.y - operand;
    registers.status_flags.carry = registers.y >= operand;

    SET_NEGATIVE_AND_ZERO(result);
}

void _execute_opcode_dec(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = operand - 1;

    SET_NEGATIVE_AND_ZERO(result);

    bus->write_cpu_byte(operand_address, result);
}

void _execute_opcode_dex(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.x--;

    SET_NEGATIVE_AND_ZERO(registers.x);
}

void _execute_opcode_dey(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.y--;

    SET_NEGATIVE_AND_ZERO(registers.y);
}

void _execute_opcode_eor(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = registers.a ^ operand;

    SET_NEGATIVE_AND_ZERO(result);

    registers.a = result;
}

void _execute_opcode_inc(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = operand + 1;

    SET_NEGATIVE_AND_ZERO(result);

    bus->write_cpu_byte(operand_address, result);
}

void _execute_opcode_inx(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.x++;

    SET_NEGATIVE_AND_ZERO(registers.x);
}

void _execute_opcode_iny(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.y++;

    SET_NEGATIVE_AND_ZERO(registers.y);
}

void _execute_opcode_lsr(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = operand >> 1;

    SET_NEGATIVE_AND_ZERO(result);

    registers.status_flags.carry = (operand & 0x1);
    bus->write_cpu_byte(operand_address, result);
}

void _execute_opcode_acc_lsr(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 result = registers.a >> 1;

    SET_NEGATIVE_AND_ZERO(result);

    registers.status_flags.carry = (registers.a & 0x1);
    registers.a = result;
}

void _execute_opcode_ora(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = registers.a | operand;

    SET_NEGATIVE_AND_ZERO(result);

    registers.a = result;
}

void _execute_opcode_rol(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = (uint8) registers.status_flags.carry | (operand << 1);

    SET_NEGATIVE_AND_ZERO(result);
    SET_CARRY(operand);

    bus->write_cpu_byte(operand_address, result);
}

void _execute_opcode_acc_rol(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 result = (uint8) registers.status_flags.carry | (registers.a << 1);

    SET_NEGATIVE_AND_ZERO(result);
    SET_CARRY(registers.a);

    registers.a = result;
}

void _execute_opcode_acc_ror(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 result = (registers.a >> 1) | (registers.status_flags.carry << 0x7);

    SET_NEGATIVE_AND_ZERO(result);

    registers.status_flags.carry = !!(registers.a & 0x1);
    registers.a = result;
}

void _execute_opcode_ror(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = (operand >> 1) | (registers.status_flags.carry << 0x7);

    SET_NEGATIVE_AND_ZERO(result);

    registers.status_flags.carry = !!(operand & 0x1);
    bus->write_cpu_byte(operand_address, result);
}

void _execute_opcode_sbc(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    uint8 result = registers.a - operand - (1 - registers.status_flags.carry);
    int16 carry_test = (int16) registers.a - operand - (1 - registers.status_flags.carry);

    SET_NEGATIVE_AND_ZERO(result);

    if (carry_test >= 0) registers.status_flags.carry = 1;
    else registers.status_flags.carry = 0;

    registers.status_flags.overflow = !SAME_SIGN(registers.a, result) && !SAME_SIGN(registers.a, operand);
    registers.a = result & 0xFF;
}

void _execute_opcode_bcc(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    if (!registers.status_flags.carry)
    {
        registers.pc = operand_address;
    }
}

void _execute_opcode_bcs(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    if (registers.status_flags.carry)
    {
        registers.pc = operand_address;
    }
}

void _execute_opcode_beq(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    if (registers.status_flags.zero)
    {
        registers.pc = operand_address;
    }
}

void _execute_opcode_bmi(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    if (registers.status_flags.negative)
    {
        registers.pc = operand_address;
    }
}

void _execute_opcode_bne(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    if (!registers.status_flags.zero)
    {
        registers.pc = operand_address;
    }
}

void _execute_opcode_bpl(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    if (!registers.status_flags.negative)
    {
        registers.pc = operand_address;
    }
}

void _execute_opcode_bvc(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    if (!registers.status_flags.overflow)
    {
        registers.pc = operand_address;
    }
}

void _execute_opcode_bvs(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    if (registers.status_flags.overflow)
    {
        registers.pc = operand_address;
    }
}

void _execute_opcode_bit(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    uint8 operand = bus->read_cpu_byte(operand_address);
    registers.status_flags.overflow = !!(operand & 0x40);

    SET_NEGATIVE(operand);
    SET_ZERO(registers.a & operand);
}

void _execute_opcode_brk(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    PUSH_STACK_SHORT(registers.pc);
    PUSH_STACK_BYTE(registers.status_byte | STATUS_BREAK_MASK);

    registers.status_flags.interrupt_disable = 1;
    registers.pc = bus->read_cpu_short(BREAK_INTERRUPT_VECTOR);
}

void _execute_opcode_jmp(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    // operand address will contain the 16 bit target address.
    registers.pc = operand_address;
}

void _execute_opcode_jsr(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    // we need to push $pc+2 but we've already advanced our counter
    // by the sie of JSR (3 bytes). We subtract one as a result.

    registers.pc--;

    PUSH_STACK_SHORT(registers.pc);

    registers.pc = operand_address;
}

void _execute_opcode_pha(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    PUSH_STACK_BYTE(registers.a);
}

void _execute_opcode_php(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    PUSH_STACK_BYTE(registers.status_byte | STATUS_BREAK_MASK);
}

void _execute_opcode_pla(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    POP_STACK_BYTE(registers.a);
    SET_NEGATIVE_AND_ZERO(registers.a);
}

void _execute_opcode_plp(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    POP_STACK_BYTE(registers.status_byte);

    registers.status_byte = 0x20 | (registers.status_byte & 0xEF);
}

void _execute_opcode_rti(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    POP_STACK_BYTE(registers.status_byte);
    POP_STACK_SHORT(registers.pc);

    registers.status_flags.break_command = 0;
    registers.status_flags.unused = 1;
}

void _execute_opcode_rts(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    POP_STACK_SHORT(registers.pc);

    registers.pc++;
}

// this is a virtual opcode that handles an interrupt request. 
void _execute_opcode_int(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    PUSH_STACK_SHORT(registers.pc);

    _execute_opcode_php(0, bus, registers);
    registers.status_flags.interrupt_disable = 1;
    registers.pc = bus->read_cpu_short(operand_address);
}

void _execute_opcode_lda(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    LOAD_REG(registers.a);

    SET_NEGATIVE_AND_ZERO(registers.a);
}

void _execute_opcode_ldx(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    LOAD_REG(registers.x);

    SET_NEGATIVE_AND_ZERO(registers.x);
}

void _execute_opcode_ldy(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    LOAD_REG(registers.y);

    SET_NEGATIVE_AND_ZERO(registers.y);
}

void _execute_opcode_sta(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    STORE_REG(registers.a);
}

void _execute_opcode_stx(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    STORE_REG(registers.x);
}

void _execute_opcode_sty(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    STORE_REG(registers.y);
}

void _execute_opcode_tax(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.x = registers.a;

    SET_NEGATIVE_AND_ZERO(registers.x);
}

void _execute_opcode_tay(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.y = registers.a;

    SET_NEGATIVE_AND_ZERO(registers.y);
}

void _execute_opcode_tsx(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.x = registers.sp;

    SET_NEGATIVE_AND_ZERO(registers.x);
}

void _execute_opcode_txa(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.a = registers.x;

    SET_NEGATIVE_AND_ZERO(registers.a);
}

void _execute_opcode_txs(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.sp = registers.x;

    // This doesn't seem to be necessary, based on the nestest.nes log.
    // SET_NEGATIVE_AND_ZERO(registers.x);
}

void _execute_opcode_tya(uint16 operand_address, system_bus *bus, cpu_register_set &registers)
{
    registers.a = registers.y;

    SET_NEGATIVE_AND_ZERO(registers.a);
}

} // namespace nes