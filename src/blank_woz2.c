#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

static const int C_QTRACK = 0x28*4;
static const int C_REAL_TRACK = 0x23;
static const int C_REAL_QTRACK = C_REAL_TRACK*4-3;
static const uint32_t bitsPerTrack = 0xD000u;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s filename.woz2\n", argv[0]);
        return 1;
    }



    FILE* woz2 = fopen(argv[1], "wb");



    fwrite("WOZ2", 1, 4, woz2);

    uint8_t highBitSet = 0xffu;
    fwrite(&highBitSet, sizeof(highBitSet), 1, woz2);

    fwrite("\n\r\n", 1, 3, woz2);

    uint32_t crc = 0;
    fwrite(&crc, sizeof(crc), 1, woz2);



    fwrite("INFO", 1, 4, woz2);
    uint32_t sizeInfo = 60;
    fwrite(&sizeInfo, sizeof(sizeInfo), 1, woz2);

    uint8_t vInfo = 2;
    fwrite(&vInfo, sizeof(vInfo), 1, woz2);

    uint8_t diskType = 1;
    fwrite(&diskType, sizeof(diskType), 1, woz2);

    uint8_t writeProtect = 0;
    fwrite(&writeProtect, sizeof(writeProtect), 1, woz2);

    uint8_t sync = 0;
    fwrite(&sync, sizeof(sync), 1, woz2);

    uint8_t cleaned = 1;
    fwrite(&cleaned, sizeof(cleaned), 1, woz2);

    char creator[32] = "blank_woz2                      ";
    fwrite(&creator, 1, 32, woz2);

    uint8_t sides = 1;
    fwrite(&sides, sizeof(sides), 1, woz2);

    uint8_t sectorFormat = 0;
    fwrite(&sectorFormat, sizeof(sectorFormat), 1, woz2);

    uint8_t timing = 4*8;
    fwrite(&timing, sizeof(timing), 1, woz2);

    uint16_t hardware = 0x01FFu;
    fwrite(&hardware, sizeof(hardware), 1, woz2);

    uint16_t ram = 0;
    fwrite(&ram, sizeof(ram), 1, woz2);

    uint16_t blocksPerTrack = (bitsPerTrack/8u + 0x200u - 1u) / 0x200u;
    fwrite(&blocksPerTrack, sizeof(blocksPerTrack), 1, woz2);

    uint8_t fill = 0;
    for (int i = 0; i < 14; ++i) {
        fwrite(&fill, sizeof(fill), 1, woz2);
    }



    fwrite("TMAP", 1, 4, woz2);
    uint32_t sizeTmap = C_QTRACK;
    fwrite(&sizeTmap, sizeof(sizeTmap), 1, woz2);

    uint8_t empty = 0xFFu;
    for (uint8_t qt = 0; qt < C_REAL_QTRACK; ++qt) {
        if ((qt+2) % 4) {
            uint8_t t = (qt+2) / 4;
            fwrite(&t, sizeof(t), 1, woz2);
        } else {
            fwrite(&empty, sizeof(empty), 1, woz2);
        }
    }
    for (uint8_t qt = C_REAL_QTRACK; qt < C_QTRACK; ++qt) {
        fwrite(&empty, sizeof(empty), 1, woz2);
    }



    fwrite("TRKS", 1, 4, woz2);
    uint32_t sizeTrks = C_QTRACK*8 + C_REAL_TRACK*blocksPerTrack*0x200u;
    fwrite(&sizeTrks, sizeof(sizeTrks), 1, woz2);
    uint16_t block = 3;
    for (uint8_t qt = 0; qt < C_REAL_TRACK; ++qt) {
        fwrite(&block, sizeof(block), 1, woz2);
        fwrite(&blocksPerTrack, sizeof(blocksPerTrack), 1, woz2);
        fwrite(&bitsPerTrack, sizeof(bitsPerTrack), 1, woz2);

        block += blocksPerTrack;
    }
    for (uint8_t qt = C_REAL_TRACK; qt < C_QTRACK; ++qt) {
        uint16_t fillBlock = 0;
        fwrite(&fillBlock, sizeof(fillBlock), 1, woz2);
        uint32_t fillCountBits = 0;
        uint16_t fillCountBlock = 0;
        fwrite(&fillCountBlock, sizeof(fillCountBlock), 1, woz2);
        fwrite(&fillCountBits, sizeof(fillCountBits), 1, woz2);
    }

    // BITS
    uint8_t *emptyBlock = (uint8_t*)calloc(0x200, 1);
    for (int b = 0; b < C_REAL_TRACK*blocksPerTrack; ++b) {
        fwrite(emptyBlock, 0x200, 1, woz2);
    }
    free(emptyBlock);

    fclose(woz2);

    return EXIT_SUCCESS;
}
