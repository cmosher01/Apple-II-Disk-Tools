#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "a2const.h"
#include "nibblize_4_4.h"
#include "nibblize_5_3.h"
#include "nibblize_5_3_alt.h"
#include "nibblize_6_2.h"



// convert DO or D13 to WOZ according to
// WOX 2.0 spec. standard

// number of quarter-tracks defined in WOZ standard
#define C_QTRACK (0x28u*4)

// number of tracks on normal floppy
#define C_REAL_TRACK 0x23u

// quarter-tracks on normal floppy: 0, 0.25, 0.5, 0.75, 1, ..., 33.5, 33.75, 34
#define C_REAL_QTRACK (C_REAL_TRACK*4-3)

// number of 512-byte blocks for each track
// (must be more than enough for the longest track we need)
//  1 block  can hold 0x1000 bits
// 14 blocks can hold 0xE000 bits
#define BLOCKS_PER_TRACK 14



static FILE *dsk = 0;
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

    char creator[32] = "to_woz2                         ";
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

    uint16_t blocksInLargestTrack = BLOCKS_PER_TRACK;
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
    for (uint8_t qt = 0; qt < C_REAL_QTRACK; ++qt) {
        if ((qt+2) % 4) {
            uint8_t t = (qt+2) / 4;
            fwrite(&t, sizeof(t), 1, woz);
        } else {
            fwrite(&empty, sizeof(empty), 1, woz);
        }
    }
    for (uint8_t qt = C_REAL_QTRACK; qt < C_QTRACK; ++qt) {
        fwrite(&empty, sizeof(empty), 1, woz);
    }
}

static void trks(uint32_t bitsPerTrack) {
    fwrite("TRKS", 1, 4, woz);
    uint32_t sizeTrks = C_QTRACK*8 + C_REAL_TRACK*BLOCKS_PER_TRACK*0x200u;
    fwrite(&sizeTrks, sizeof(sizeTrks), 1, woz);
    uint16_t block = 3;
    for (uint8_t qt = 0; qt < C_REAL_TRACK; ++qt) {
        fwrite(&block, sizeof(block), 1, woz);
        uint16_t blocksPerTrack = BLOCKS_PER_TRACK;
        fwrite(&blocksPerTrack, sizeof(blocksPerTrack), 1, woz);
        fwrite(&bitsPerTrack, sizeof(bitsPerTrack), 1, woz);

        block += BLOCKS_PER_TRACK;
    }
    for (uint8_t qt = C_REAL_TRACK; qt < C_QTRACK; ++qt) {
        uint16_t fillBlock = 0;
        fwrite(&fillBlock, sizeof(fillBlock), 1, woz);
        uint32_t fillCountBits = 0;
        uint16_t fillCountBlock = 0;
        fwrite(&fillCountBlock, sizeof(fillCountBlock), 1, woz);
        fwrite(&fillCountBits, sizeof(fillCountBits), 1, woz);
    }
}





static uint8_t mp_sector13[13];

static void build_mp_sector13() {
    uint8_t s = 0;
    for (uint_fast8_t i = 0; i < 13; ++i) {
        mp_sector13[i] = s;
        s += 10;
        s %= 13;
    }
}

static uint8_t map_sector_13(uint8_t s) {
    return mp_sector13[s];
}

static const uint8_t mp_sector16do[] = { 0x0, 0x7, 0xE, 0x6, 0xD, 0x5, 0xC, 0x4, 0xB, 0x3, 0xA, 0x2, 0x9, 0x1, 0x8, 0xF };
static const uint8_t mp_sector16po[] = { 0x0, 0x8, 0x1, 0x9, 0x2, 0xA, 0x3, 0xB, 0x4, 0xC, 0x5, 0xD, 0x6, 0xE, 0x7, 0xF };

static uint8_t map_sector_do16(uint8_t s) {
    return mp_sector16do[s];
}

static long int sector_offset(uint_fast8_t dos33, uint8_t track, uint8_t sector) {
    if (dos33) {
        return (track * 16 * BYTES_PER_SECTOR) + (map_sector_do16(sector) * BYTES_PER_SECTOR);
    }
    return (track * 13 * BYTES_PER_SECTOR) + (map_sector_13(sector) * BYTES_PER_SECTOR);
}



static uint8_t trk[BLOCKS_PER_TRACK*0x200u];

static void clear() {
    memset(trk, 0, BLOCKS_PER_TRACK*0x200u);
}

static uint32_t writeBit(const uint32_t i, const uint_fast8_t x) {
    const uint32_t ibyt = i >> 3;
    const uint8_t m = 0x80u >> (i & 7u);
    if (x) {
        trk[ibyt] |= m;
    } else {
        trk[ibyt] &= ~m;
    }
    return i+1;
}

static uint32_t writeByte(uint32_t i, uint8_t x) {
    for (uint_fast8_t bit = 0; bit < 8; ++bit) {
        i = writeBit(i, x & 0x80u);
        x <<= 1;
    }
    return i;
}

static uint32_t writeWord(uint32_t i, uint16_t x) {
    i = writeByte(i, x & 0xFFu);
    x >>= 8;
    i = writeByte(i, x & 0xFFu);
    return i;
}

static uint32_t writeSyncByte(uint32_t i, uint_fast8_t cExtraBits) {
    i = writeByte(i, 0xFFu);
    while (cExtraBits--) {
        i = writeBit(i, 0);
    }
    return i;
}

/*
 * trak: gap   sect+
 * sect: addr gap   data gap
 * addr: head_marker addr_id   v t s   k   tail_marker tail_id
 * data: head_marker data_id   nibl+   k   tail_marker tail_id
 */

/*
For reference, this is what a 13-sector INIT tries to write:

9808 * FF36 (= 88272 bits = 0x158D0) (about 1.5 revolutions)

13 *
{
    79 * FF36 (= 711 bits)
    FF32
    D5 AA B5 FF FE AA AA AA AA FF FE DE AA EB (= 120 bits)
    431 * FF32 (= 3448 bits)
    (= 4279 bits)
} (= 55627 bits = 0xD94B) (= 6954 8-bit bytes = 0x1B2A)
*/
static uint32_t writeSyncGap(uint32_t i, uint_fast16_t cBytes, const uint_fast8_t cExtraBits) {
    while (cBytes--) {
        i = writeSyncByte(i, cExtraBits);
    }
    return i;
}

static uint32_t writeHeadMarker(uint32_t i) {
    i = writeWord(i, 0xAAD5u);
    return i;
}

static uint32_t writeTailMarker(uint32_t i) {
    i = writeWord(i, 0xAADEu);
    return i;
}

static uint32_t writeAddrId(uint32_t i, uint_fast8_t dos33) {
    if (dos33) {
        i = writeByte(i, 0x96u);
    } else {
        i = writeByte(i, 0xB5u);
    }
    return i;
}

static uint32_t writeDataId(uint32_t i) {
    i = writeByte(i, 0xADu);
    return i;
}

static uint32_t writeTailID(uint32_t i) {
    i = writeByte(i, 0xEBu);
    return i;
}

static uint32_t writeAddr(uint32_t i, uint8_t volume, uint8_t track, uint8_t sector, uint_fast8_t dos33) {
    i = writeHeadMarker(i);
    i = writeAddrId(i, dos33);

    if (!dos33) {
        sector = map_sector_13(sector);
    }
    i = writeWord(i, nibblize_4_4_encode(volume));
    i = writeWord(i, nibblize_4_4_encode(track));
    i = writeWord(i, nibblize_4_4_encode(sector));
    i = writeWord(i, nibblize_4_4_encode(volume ^ track ^ sector));

    i = writeTailMarker(i);
    i = writeTailID(i);
    return i;
}

enum { ENC_62, ENC_53, ENC_53A };

static uint32_t writeEncoded(uint32_t i, const uint8_t* unencodedSectorData, uint_fast8_t enc) {
    uint8_t *encoded = calloc(0x200, 1);
    uint8_t *encoded_start = encoded;
    switch (enc) {
    case ENC_62:
        nibblize_6_2_encode(&unencodedSectorData, &encoded);
        break;
    case ENC_53:
        nibblize_5_3_encode(&unencodedSectorData, &encoded);
        break;
    case ENC_53A:
        nibblize_5_3_alt_encode(&unencodedSectorData, &encoded);
        break;
    }
    for (int x = 0; x < encoded-encoded_start; ++x) {
        i = writeByte(i, encoded_start[x]);
    }
    free(encoded_start);
    return i;
}

static uint32_t writeData(uint32_t i, const uint8_t* unencodedSectorData, uint_fast8_t enc) {
    i = writeHeadMarker(i);
    i = writeDataId(i);

    i = writeEncoded(i, unencodedSectorData, enc);

    i = writeTailMarker(i);
    i = writeTailID(i);
    return i;
}


static uint_fast8_t deduce_encoding(uint_fast8_t dos33, uint8_t track, uint8_t sector) {
    if (dos33) {
        return ENC_62;
    }
    if (track || sector) {
        return ENC_53;
    }
    return ENC_53A;
}

static uint8_t sector[BYTES_PER_SECTOR];

static void bits(uint_fast8_t dos33) {
    for (uint8_t t = 0; t < C_REAL_TRACK; ++t) {
        printf("track $%02X: ", t);
        clear();
        if (dsk) {
            uint32_t i = 0;
            i = writeSyncGap(i, 0x20u, 1+dos33);
            for (uint8_t s = 0; s < (dos33 ? 16 : 13); ++s) {
                printf("%1X ", s);
                memset(sector, 0, BYTES_PER_SECTOR);
                fseek(dsk, sector_offset(dos33, t, s), SEEK_SET);
                size_t c_read = fread(sector, 1, BYTES_PER_SECTOR, dsk);
                if (c_read < BYTES_PER_SECTOR) {
                    fprintf(stderr, "not enough input bytes, filling with zeroes\n");
                }

                i = writeAddr(i, 0xFEu, t, s, dos33);
                i = writeSyncGap(i, 0x08u, 1+dos33);
                i = writeData(i, sector, deduce_encoding(dos33, t, s));
                i = writeSyncGap(i, 0x10u, 1+dos33);
            }
        } else {
            printf("[uninitialized]");
        }
        printf("\n");
        fwrite(trk, 0x200, BLOCKS_PER_TRACK, woz);
    }
}



// strip trailing ":.dsk"
static char *parse_filename(char *arg) {
    char *filename = malloc(strlen(arg));
    strcpy(filename, arg);
    char *colon = strchr(filename, ':');
    if (colon) {
        *colon = 0;
    }
    return filename;
}



// 0=13-sector, 1=16-sector
static uint_fast8_t deduce_filetype_from_name(char *arg) {
    char *dot = strrchr(arg, '.');
    if (dot) {
        if (!strcmp(dot, ".dsk")) {
            return 1;
        }
        if (!strcmp(dot, ".do")) {
            return 1;
        }
        if (!strcmp(dot, ".d13")) {
            return 0;
        }
    }
    fprintf(stderr, "ERROR: cannot determine file type of file %s\n", arg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2 || 3 < argc) {
        printf("usage: %s [input.dsk] output.woz\n", argv[0]);
        printf("Converts a DOS 13- or 16-sector Apple ][ disk image to WOZ 2.0.\n");
        printf("input.dsk is the input disk image.\n");
        printf("If omitted, then a blank image is output (35 tracks all zeroes).\n");
        printf("For a blank 13-sector image, use \":.d13\".\n");
        printf("File types are determined from the name:\n");
//        printf("    .woz         WOZ 2.0 output file\n");
        printf("    .dsk or .do  16-sector DOS order (143360 bytes)\n");
        printf("    .d13         13-sector DOS order (116480 bytes)\n");
        printf("To force the type, append colon and type to the file name.\n");
        printf("For example: \"somefile.foo:.dsk\" .\n");
        return 1;
        // TODO handle ProDOS-order sectors
        // TODO handle multiple input files
        // TODO optional output file (deduce output file name from input file name)
    }



    char *name_dsk = 0;
    char *name_woz = 0;
    if (argc == 3) {
        name_dsk = argv[1];
        name_woz = argv[2];
    } else {
        name_woz = argv[1];
    }

    // make sure output file does not already exist, for safety
    woz = fopen(name_woz, "rb");
    if (woz) {
        fclose(woz);
        fprintf(stderr, "ERROR: %s exists; will NOT overwrite existing output file\n", name_woz);
        exit(1);
    }

    uint_fast8_t dos33 = 1;
    if (name_dsk) {
        dos33 = deduce_filetype_from_name(name_dsk);
        printf("%s filetype: %d-sector\n", name_dsk, (dos33 ? 16 : 13));

        char *filename = parse_filename(name_dsk);
        if (*filename) {
            dsk = fopen(filename, "rb");
            if (dsk == 0) {
                fprintf(stderr, "ERROR: cannot open input file %s\n", name_dsk);
                exit(1);
            }
        }
        free(filename);

        build_mp_sector13();
    }

    uint32_t bits_per_track;
    if (dsk) {
        if (dos33) {
            bits_per_track = 0xC5C0u;
        } else {
            bits_per_track = 0xBB30;
        }
    } else {
        bits_per_track = 0xE000u;
    }

    woz = fopen(name_woz, "wb");



    header(0); // TODO calculate CRC
    info(1+dos33);
    tmap();
    trks(bits_per_track);
    bits(dos33);



    fclose(dsk);
    fclose(woz);



    return EXIT_SUCCESS;
}
