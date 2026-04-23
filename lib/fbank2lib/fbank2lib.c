#pragma once

#include "fbank2lib.h"

void fb2_disable_wr_protection() {
    FLASH->KEYR2 = 0x45670123;
    FLASH->KEYR2 = 0xCDEF89AB;

    if (!(FLASH->CR2 & FLASH_CR_LOCK))
        printf("\r\nFlash bank 2 is unlocked!");
    else
        printf("\r\nUnable to unlock flash bank 2! (CR2 was %d)", FLASH->CR2 & FLASH_CR_LOCK);

    printf("\r\nPG before write: %d", FLASH->CR2 & FLASH_CR_PG);
    FLASH->CR2 |= FLASH_CR_PG;
    printf("\r\nPG after write: %d", FLASH->CR2 & FLASH_CR_PG);
}

uint32_t fb2_read_word(uint32_t offset) {
    return *(uint32_t*)(FLASH_BANK2_BASE + offset);
}

void fb2_write_word(uint32_t offset, uint32_t data) {
    *(uint32_t*)(FLASH_BANK2_BASE + offset) = data;
}

void fb2_write(uint32_t * data, uint32_t size, uint32_t base_offset) {
    for (uint32_t i = 0; i < size; i++)
    {
        fb2_write_word(base_offset+i, data[i]);
    }
}