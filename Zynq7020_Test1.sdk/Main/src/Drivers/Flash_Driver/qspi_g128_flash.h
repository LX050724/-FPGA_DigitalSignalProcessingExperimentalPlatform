#ifndef QSPI_G128_FLASH_H_
#define QSPI_G128_FLASH_H_

#include "xqspips.h"

int Init_qspi(XQspiPs *QspiInstancePtr, uint16_t QspiDeviceId);
void update_flash(uint8_t *buffer, uint8_t *read_buffer, uint8_t *write_buffer, uint32_t length);
void FlashErase(XQspiPs *QspiPtr, uint32_t Address, uint32_t ByteCount, uint8_t *WriteBfrPtr);

void FlashWrite(XQspiPs *QspiPtr, uint32_t Address, uint32_t ByteCount, uint8_t Command, uint8_t *WriteBfrPtr);

int FlashReadID(XQspiPs *QspiPtr, uint8_t *WriteBfrPtr, uint8_t *ReadBfrPtr);

void FlashRead(XQspiPs *QspiPtr, uint32_t Address, uint32_t ByteCount, uint8_t Command, uint8_t *WriteBfrPtr,
               uint8_t *ReadBfrPtr);

int SendBankSelect(XQspiPs *QspiPtr, uint8_t *WriteBfrPtr, uint32_t BankSel);

void BulkErase(XQspiPs *QspiPtr, uint8_t *WriteBfrPtr);

void DieErase(XQspiPs *QspiPtr, uint8_t *WriteBfrPtr);

uint32_t GetRealAddr(XQspiPs *QspiPtr, uint32_t Address);

#endif /* QSPI_G128_FLASH_H_ */
