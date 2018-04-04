#include "../md2.h"
#include <memory.h>

#define MD2_BLOCKSIZE 16

const uint8_t accelc_MD2_PI_SUBST[256] = {
    0x29, 0x2E, 0x43, 0xC9, 0xA2, 0xD8, 0x7C, 0x01, 0x3D, 0x36, 0x54, 0xA1, 0xEC, 0xF0, 0x06, 0x13,
    0x62, 0xA7, 0x05, 0xF3, 0xC0, 0xC7, 0x73, 0x8C, 0x98, 0x93, 0x2B, 0xD9, 0xBC, 0x4C, 0x82, 0xCA,
    0x1E, 0x9B, 0x57, 0x3C, 0xFD, 0xD4, 0xE0, 0x16, 0x67, 0x42, 0x6F, 0x18, 0x8A, 0x17, 0xE5, 0x12,
    0xBE, 0x4E, 0xC4, 0xD6, 0xDA, 0x9E, 0xDE, 0x49, 0xA0, 0xFB, 0xF5, 0x8E, 0xBB, 0x2F, 0xEE, 0x7A,
    0xA9, 0x68, 0x79, 0x91, 0x15, 0xB2, 0x07, 0x3F, 0x94, 0xC2, 0x10, 0x89, 0x0B, 0x22, 0x5F, 0x21,
    0x80, 0x7F, 0x5D, 0x9A, 0x5A, 0x90, 0x32, 0x27, 0x35, 0x3E, 0xCC, 0xE7, 0xBF, 0xF7, 0x97, 0x03,
    0xFF, 0x19, 0x30, 0xB3, 0x48, 0xA5, 0xB5, 0xD1, 0xD7, 0x5E, 0x92, 0x2A, 0xAC, 0x56, 0xAA, 0xC6,
    0x4F, 0xB8, 0x38, 0xD2, 0x96, 0xA4, 0x7D, 0xB6, 0x76, 0xFC, 0x6B, 0xE2, 0x9C, 0x74, 0x04, 0xF1,
    0x45, 0x9D, 0x70, 0x59, 0x64, 0x71, 0x87, 0x20, 0x86, 0x5B, 0xCF, 0x65, 0xE6, 0x2D, 0xA8, 0x02,
    0x1B, 0x60, 0x25, 0xAD, 0xAE, 0xB0, 0xB9, 0xF6, 0x1C, 0x46, 0x61, 0x69, 0x34, 0x40, 0x7E, 0x0F,
    0x55, 0x47, 0xA3, 0x23, 0xDD, 0x51, 0xAF, 0x3A, 0xC3, 0x5C, 0xF9, 0xCE, 0xBA, 0xC5, 0xEA, 0x26,
    0x2C, 0x53, 0x0D, 0x6E, 0x85, 0x28, 0x84, 0x09, 0xD3, 0xDF, 0xCD, 0xF4, 0x41, 0x81, 0x4D, 0x52,
    0x6A, 0xDC, 0x37, 0xC8, 0x6C, 0xC1, 0xAB, 0xFA, 0x24, 0xE1, 0x7B, 0x08, 0x0C, 0xBD, 0xB1, 0x4A,
    0x78, 0x88, 0x95, 0x8B, 0xE3, 0x63, 0xE8, 0x6D, 0xE9, 0xCB, 0xD5, 0xFE, 0x3B, 0x00, 0x1D, 0x39,
    0xF2, 0xEF, 0xB7, 0x0E, 0x66, 0x58, 0xD0, 0xE4, 0xA6, 0x77, 0x72, 0xF8, 0xEB, 0x75, 0x4B, 0x0A,
    0x31, 0x44, 0x50, 0xB4, 0x8F, 0xED, 0x1F, 0x1A, 0xDB, 0x99, 0x8D, 0x33, 0x9F, 0x11, 0x83, 0x14
};

void accelc_MD2_init(MD2_DIGEST* HashBuffer) {
    HashBuffer->dword[0] = 0;
    HashBuffer->dword[1] = 0;
    HashBuffer->dword[2] = 0;
    HashBuffer->dword[3] = 0;
}

void accelc_MD2_main_cycle(const void* srcBytes, size_t srcBytesLength, MD2_DIGEST* HashBuffer) {
    size_t RoundsOfMainCycle = srcBytesLength / MD2_BLOCKSIZE;
    const uint64_t (*MessageBlock)[MD2_BLOCKSIZE / sizeof(uint64_t)] = (const uint64_t (*)[MD2_BLOCKSIZE / sizeof(uint64_t)])srcBytes;
    union {
        uint8_t byte[3 * MD2_BLOCKSIZE];
        uint64_t qword[3 * MD2_BLOCKSIZE / sizeof(uint64_t)];
    } Buffer;

    memcpy(&Buffer, HashBuffer, sizeof(MD2_DIGEST));

    for (size_t i = 0; i < RoundsOfMainCycle; ++i) {
        for (int j = 0; j < 16; ++j) {
            Buffer.qword[2] = MessageBlock[i][0];
            Buffer.qword[3] = MessageBlock[i][1];
            Buffer.qword[4] = Buffer.qword[2] ^ Buffer.qword[0];
            Buffer.qword[5] = Buffer.qword[3] ^ Buffer.qword[1];
        }

        uint8_t t = 0;
        for (int j = 0; j < 18; ++j) {
            for (int k = 0; k < 48; ++k) {
                Buffer.byte[k] ^= accelc_MD2_PI_SUBST[t];
                t = Buffer.byte[k];
            }
            t += j;
        }
    }

    memcpy(HashBuffer, &Buffer, sizeof(MD2_DIGEST));
}

void accelc_MD2_final(const void* srcBytes, size_t srcBytesLength, MD2_DIGEST* HashBuffer) {
    size_t RoundsOfMainCycle = srcBytesLength / 16;
    size_t LeftBytesLength = srcBytesLength % 16;
    const uint8_t (*Ptr)[16] = (const uint8_t (*)[16])srcBytes;
    uint8_t Extra[32] = { 0 };

    for (size_t i = 0; i < LeftBytesLength; ++i)
        Extra[i] = Ptr[RoundsOfMainCycle][i];

    for (uint8_t padding = (uint8_t)(16 - LeftBytesLength), i = 0; i < padding; ++i)
        Extra[LeftBytesLength + i] = padding;

    uint8_t L = 0;
    for (size_t i = 0; i < RoundsOfMainCycle; ++i) {
        for (int j = 0; j < 16; ++j) {
            Extra[16 + j] ^= accelc_MD2_PI_SUBST[Ptr[i][j] ^ L];	// I do not know if I misunderstand RFC 1319.
                                                                    // According to RFC 1319, page 3, " Set C[j] to S[c xor L]. " is written in section "Step 2. Append Checksum".
                                                                    // Here, Extra[16 + j] is C[j]. Ptr[i][j] is c and PI_SUBST is S.
                                                                    // But it is "Set ... to ...", so "Extra[16 + j]" should BE "PI_SUBST[Ptr[i][j] ^ L]".
                                                                    // However, the corresponding code that is attached to RFC 1319 document, is "t = x[j] ^= PI_SUBST[t];"
                                                                    // It is "^=", not "=", WHY????
            L = Extra[16 + j];
        }
    }
    for (int j = 0; j < 16; ++j) {
        Extra[16 + j] ^= accelc_MD2_PI_SUBST[Extra[j] ^ L];
        L = Extra[16 + j];
    }

    accelc_MD2_main_cycle(Extra, 32, HashBuffer);
}

void accelc_MD2(const void* srcBytes, size_t srclen, MD2_DIGEST* Hash) {
    accelc_MD2_init(Hash);
    accelc_MD2_main_cycle(srcBytes, srclen, Hash);
    accelc_MD2_final(srcBytes, srclen, Hash);
}