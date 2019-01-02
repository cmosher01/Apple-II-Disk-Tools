#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



// create empty WOZ (no tracks) according to
// WOX 2.0 spec. standard

// number of quarter-tracks defined in WOZ standard
#define C_QTRACK (0x28u*4)



static FILE *woz = 0;



static void header(uint32_t crc) {
    fwrite("WOZ2", 1, 4, woz);

    uint8_t highBitSet = 0xFFu;
    fwrite(&highBitSet, sizeof(highBitSet), 1, woz);

    fwrite("\n\r\n", 1, 3, woz);

    fwrite(&crc, sizeof(crc), 1, woz);
}

// sectorFormat: 0=unk, 1=13-s, 2=16-s, 3=both
static void info(const uint8_t sectorFormat) {
    fwrite("INFO", 1, 4, woz);
    uint32_t sizeInfo = 60;
    fwrite(&sizeInfo, sizeof(sizeInfo), 1, woz);

    uint8_t vInfo = 2; // WOZ 2.0
    fwrite(&vInfo, sizeof(vInfo), 1, woz);

    uint8_t diskType = 1; // 5.25-inch disk
    fwrite(&diskType, sizeof(diskType), 1, woz);

    uint8_t writeProtect = 0; // not write protected
    fwrite(&writeProtect, sizeof(writeProtect), 1, woz);

    uint8_t sync = 0; // tracks not synchronized
    fwrite(&sync, sizeof(sync), 1, woz);

    uint8_t cleaned = 1; // randomly generated bits not included
    fwrite(&cleaned, sizeof(cleaned), 1, woz);

    char creator[32] = "empty_woz2                      ";
    fwrite(&creator, 1, 32, woz);

    uint8_t sides = 1; // always 1 for 5.25-inch disks
    fwrite(&sides, sizeof(sides), 1, woz);

    fwrite(&sectorFormat, sizeof(sectorFormat), 1, woz);

    uint8_t timing = 4*8; // normal 4us timing = 32
    fwrite(&timing, sizeof(timing), 1, woz);

    uint16_t hardware = 0x01FFu; // bitmap (0=unknown, 1FF=all)
    fwrite(&hardware, sizeof(hardware), 1, woz);

    uint16_t ramK = 0; // minimum RAM, 0=unk
    fwrite(&ramK, sizeof(ramK), 1, woz);

    uint16_t blocksInLargestTrack = 0;
    fwrite(&blocksInLargestTrack, sizeof(blocksInLargestTrack), 1, woz);

    uint8_t fill = 0;
    for (int i = 0; i < 14; ++i) {
        fwrite(&fill, sizeof(fill), 1, woz);
    }
}

static void tmap() {
    fwrite("TMAP", 1, 4, woz);
    uint32_t sizeTmap = C_QTRACK;
    fwrite(&sizeTmap, sizeof(sizeTmap), 1, woz);

    uint8_t empty = 0xFFu;
    for (uint8_t qt = 0; qt < C_QTRACK; ++qt) {
        fwrite(&empty, sizeof(empty), 1, woz);
    }
}

static void trks() {
    fwrite("TRKS", 1, 4, woz);
    uint32_t sizeTrks = C_QTRACK*8;
    fwrite(&sizeTrks, sizeof(sizeTrks), 1, woz);

    for (uint8_t qt = 0; qt < C_QTRACK; ++qt) {
        uint16_t fillBlock = 0;
        fwrite(&fillBlock, sizeof(fillBlock), 1, woz);
        uint16_t fillCountBlock = 0;
        fwrite(&fillCountBlock, sizeof(fillCountBlock), 1, woz);
        uint32_t fillCountBits = 0;
        fwrite(&fillCountBits, sizeof(fillCountBits), 1, woz);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2 || 3 < argc) {
        printf("usage: %s output.woz\n", argv[0]);
        printf("Creates an empty WOZ 2.0 Apple ][ disk image.\n");
        return 1;
    }



    char *name_woz = argv[1];

    // make sure output file does not already exist, for safety
    woz = fopen(name_woz, "rb");
    if (woz) {
        fclose(woz);
        fprintf(stderr, "ERROR: %s exists; will NOT overwrite existing output file\n", name_woz);
        exit(1);
    }

    woz = fopen(name_woz, "wb");



    header(0); // TODO calculate CRC
    info(0);
    tmap();
    trks();



    fclose(woz);



    return EXIT_SUCCESS;
}
