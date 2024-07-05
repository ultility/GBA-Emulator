#include <check.h>
#include "cpu.h"

struct cpu cpu;

void setup(void)
{
    cpu_init(&cpu);
}

void teardown(void)
{
    free_cpu(&cpu);
}

START_TEST(check_overflow)
{
    // addition check
    ck_assert_int_eq(test_overflow(INT32_MAX, 1), true);
    ck_assert_int_eq(test_overflow(INT32_MIN, -1), true);
    // no overflow
    ck_assert_int_eq(test_overflow(5, 29), false);
    ck_assert_int_eq(test_overflow(29, 5), false);
    ck_assert_int_eq(test_overflow(0, 0), false);
    ck_assert_int_eq(test_overflow(-25, 25), false);
    ck_assert_int_eq(test_overflow(-25, -25), false);
}
END_TEST

START_TEST(check_read_write)
{
    // read write word
    
    // little endian
    cpu.registers[CPSR] &= ~E_MASK;
    write_word_to_memory(&cpu, 0x100, 0x12345678);
    ck_assert_int_eq(read_word_from_memory(&cpu, 0x100), 0x12345678);
    // big endian
    cpu.registers[CPSR] |= E_MASK;
    write_word_to_memory(&cpu, 0x100, 0x12345678);
    ck_assert_int_eq(read_word_from_memory(&cpu, 0x100), 0x12345678);

    // read write half word
    cpu.registers[CPSR] &= ~E_MASK;
    write_half_word_to_memory(&cpu, 0x100, 0x1234);
    ck_assert_int_eq(read_half_word_from_memory(&cpu, 0x100), 0x1234);
    // big endian
    cpu.registers[CPSR] |= E_MASK;
    write_half_word_to_memory(&cpu, 0x100, 0x1234);
    ck_assert_int_eq(read_half_word_from_memory(&cpu, 0x100), 0x1234);
}
END_TEST

int main(void)
{
    int number_failed = 0;
    SRunner *sr;
    Suite *s = suite_create("CPU");
    TCase *tc_core = tcase_create("Core");
    tcase_add_test(tc_core, check_overflow);
    tcase_add_test(tc_core, check_read_write);
    suite_add_tcase(s, tc_core);
    sr = srunner_create(s);

    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_set_log(sr, "check_cpu.log");
    srunner_set_xml(sr, "check_cpu.xml");
    srunner_run_all(sr, CK_VERBOSE);

    srunner_free(sr);
}