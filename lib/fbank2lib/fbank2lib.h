#pragma once

#include <stm32h7xx.h>
#include <stdio.h>

void fb2_disable_wr_protection();

uint32_t fb2_read_word(uint32_t offset);

void fb2_write_word(uint32_t data, uint32_t offset);

uint32_t fb2_write(char * data, uint32_t size, uint32_t base_offset);

uint32_t fb2_read(char * buffer, uint32_t size, uint32_t offset);

int unlock_flash_cr(uint8_t bank);
int lock_flash_cr(uint8_t bank);
int enable_write_op(uint8_t bank);
int disable_write_op(uint8_t bank);

int flash_bank_erase_seq(uint8_t bank);

// TODO записывать ожидаемую последовательность байт в флеш внутри самой программы (прошивки), чтобы локализировать проблему:
// В UART или в функциях записи?


int fb2_erase();    