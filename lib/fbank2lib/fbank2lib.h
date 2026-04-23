#pragma once

#include <stm32h7xx.h>

void fb2_disable_wr_protection();

uint32_t fb2_read_word(uint32_t offset);

void fb2_write_word(uint32_t data, uint32_t offset);

uint32_t fb2_write(char * data, uint32_t size, uint32_t base_offset);

uint32_t fb2_read(char * buffer, uint32_t size, uint32_t offset);