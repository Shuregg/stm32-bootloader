#include "fbank2lib.h"

void fb2_disable_wr_protection() {
    FLASH->KEYR2 = 0x45670123;
    FLASH->KEYR2 = 0xCDEF89AB;

    if (!(FLASH->CR2 & FLASH_CR_LOCK))
        printf("\r\nFlash bank 2 is unlocked!");
    else
        printf("\r\nUnable to unlock flash bank 2! (CR2 was %ld)", FLASH->CR2 & FLASH_CR_LOCK);

    printf("\r\nPG before write: %ld", FLASH->CR2 & FLASH_CR_PG);
    FLASH->CR2 |= FLASH_CR_PG;
    printf("\r\nPG after write: %ld", FLASH->CR2 & FLASH_CR_PG);
}

uint32_t fb2_read_word(uint32_t offset) {
    return *(uint32_t*)(FLASH_BANK2_BASE + offset);
}

// TODO implement both FLASH banks writing
void fb2_write_word(uint32_t data, uint32_t offset) {
    assert((offset & 0b11) == 00);
    *(uint32_t*)(FLASH_BANK2_BASE + offset) = data;
    #if 0
    printf("\n\r[0x%lx] wr 0x%8lx", (FLASH_BANK2_BASE + offset), data);
    #endif
}

uint32_t fb2_write(char * data, uint32_t size, uint32_t base_offset) {
    if (size % 4)
        return 1;

    uint32_t * data_word = (uint32_t*)data;
    size = size / 4;

    for (uint32_t i = 0; i < size; i++)
    {
        fb2_write_word(data_word[i], base_offset+i);
    }
}

uint32_t fb2_read(char * buffer, uint32_t size, uint32_t offset) {
    if (size % 4)
        return 1;

    uint32_t * buffer_word = (uint32_t*)buffer;
    size = size / 4;

    for (uint32_t i = 0; i < size; i++)
    {
        buffer_word[i] = fb2_read_word(offset+i);
    }
}

int unlock_flash_cr(uint8_t bank) {
    int status = 0;

    switch(bank) {
    case 1: {
        FLASH->KEYR1 = 0x45670123;
        FLASH->KEYR1 = 0xCDEF89AB;
        break;
    }
    case 2: {
        FLASH->KEYR2 = 0x45670123;
        FLASH->KEYR2 = 0xCDEF89AB;
        break;
    }
    default: {
        status = 1;
        break;
    }
    }
    return status;
}


int lock_flash_cr(uint8_t bank) {
    int status = 0;
    switch(bank) {
    case 1: {
        FLASH->CR1 |= FLASH_CR_LOCK;
        status = !(FLASH->CR1 & FLASH_CR_LOCK_Msk);
        break;
    }
    case 2: {
        FLASH->CR2 |= FLASH_CR_LOCK;
        status = !(FLASH->CR2 & FLASH_CR_LOCK_Msk);
        break;
    }
    default: {
        status = 1;
        break;
    }
    }
    return status;
}

int enable_write_op(uint8_t bank) {
    int status = 0;

    switch(bank) {
    case 1: {
        FLASH->CR1 |= FLASH_CR_PG;
        status = (FLASH->CR1 & FLASH_CR_LOCK_Msk) || !(FLASH->CR1 & FLASH_CR_PG_Msk);
        break;
    }
    case 2: {
        FLASH->CR2 |= FLASH_CR_PG;
        status = (FLASH->CR2 & FLASH_CR_LOCK_Msk) || !(FLASH->CR2 & FLASH_CR_PG_Msk);
        break;
    }
    default: {
        status = 1;
        break;
    }
    }
    return status;
}

// TODO
int disable_write_op(uint8_t bank) {

}


// TODO Flash sector erase sequence
int flash_sector_erase_seq() {

}

// Standard Flash bank erase sequence
int flash_bank_erase_seq(u_int8_t bank) {
    // 1. Check and clear (optional) all the error flags 
    // due to previous programming/erase operation.
    // Refer to Section 4.7: FLASH error management for details.
    // (Or Table 26. Flash interrupt request)
    FLASH->CCR2 = FLASH_CCR_CLR_CRCRDERR |
                  FLASH_CCR_CLR_CRCEND |
                  FLASH_CCR_CLR_DBECCERR |
                  FLASH_CCR_CLR_SNECCERR |
                  FLASH_CCR_CLR_RDSERR |
                  FLASH_CCR_CLR_RDPERR |
                  FLASH_CCR_CLR_OPERR |
                  FLASH_CCR_CLR_INCERR |
                  FLASH_CCR_CLR_STRBERR |
                  FLASH_CCR_CLR_PGSERR |
                  FLASH_CCR_CLR_WRPERR |
                  FLASH_CCR_CLR_EOP; // optional

    // 2. Unlock the FLASH_CR1/2 register, as described in Section 4.5.1:
    // FLASH configuration protection
    // (only if register is not already unlocked)
    unlock_flash_cr(bank);

    // 3. Set the BER1/2 bit in the FLASH_CR1/2 register
    // corresponding to the targeted bank.
    FLASH->CR2 |= FLASH_CR_BER; // (Bank Erase Request)

    // 4. Set the START1/2 bit in the FLASH_CR1/2 register to 
    // start the bank erase operation.
    // Then wait until the QW1/2 bit is cleared in the 
    // corresponding FLASH_SR1/2 register.
    FLASH->CR2 |= FLASH_CR_START;

    wait_qw_is_0(bank);
    
    // Note: BER1/2 and START1/2 bits can be set together, so above steps 3 and 4 can be merged.
    // If a sector erase is requested simultaneously to the bank erase (SER1/2 bit set), the bank
    // erase operation supersedes the sector erase operation.

    lock_flash_cr(bank);
}

// TODO implement both FLASH banks writing
int single_write_seq(uint8_t bank, uint32_t offset, uint32_t word) {
    // printf("\n\rFB%0u WR 0x%lx to 0x%lx", bank, word, offset);
    unlock_flash_cr(bank);
    enable_write_op(bank);
    fb2_write_word(/*bank,*/ word, offset);
    wait_qw_is_0(bank);
    lock_flash_cr(bank);
}

void wait_qw_is_0(uint8_t bank) {
    switch (bank)
    {
    case 1:
        while(FLASH->SR1 & FLASH_SR_QW_Msk);
        break;
    case 2:
        while(FLASH->SR2 & FLASH_SR_QW_Msk);
        break;
    }
}