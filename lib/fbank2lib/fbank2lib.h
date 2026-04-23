#pragma once

#include <stm32h7xx.h>

void fb2_disable_wr_protation();

uint32_t fb2_read_word(uint32_t offset);

void fb2_write_word(uint32_t offset, uint32_t data);

void fb2_write(uint32_t * data, uint32_t size, uint32_t base_offset);