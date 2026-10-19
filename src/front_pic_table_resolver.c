#include "defines.h"
#include "../include/graphics.h"

// Keep these keys in sync with scripts/insert.py and the CFRU resolver.
#define FRONT_PIC_TABLE_ADD_KEY1 0x0142B16Du
#define FRONT_PIC_TABLE_SUB_KEY1 0x02F318C9u
#define FRONT_PIC_TABLE_ADD_KEY2 0x0006D2F1u
#define FRONT_PIC_TABLE_SUB_KEY2 0x00D1A75Du
#define FRONT_PIC_TABLE_LEGACY_KEY 0x02000000u
#define ROM_PTR_START 0x08000000u
#define ROM_PTR_END   0x0A000000u

extern void LoadSpecialPokePic(const struct CompressedSpriteSheet *src, void *dest,
                               u16 species, u32 personality, bool8 isFrontPic);

void __attribute__((long_call)) DuplicateDeoxysTiles(void *pointer, s32 species);

static const struct CompressedSpriteSheet *GetEncodedFrontPicTable(void)
{
    const struct CompressedSpriteSheet *table;

    asm volatile ("ldr %0, =(gMonFrontPicTable + 0x0142B16D - 0x02F318C9 + 0x0006D2F1 - 0x00D1A75D)" : "=r"(table));
    return table;
}

static bool8 IsRomPtr(u32 ptr)
{
    return ptr >= ROM_PTR_START && ptr < ROM_PTR_END;
}

static u32 DecodeFrontPicTablePtrValue(u32 ptr)
{
    u32 decoded;

    if (IsRomPtr(ptr))
        return ptr;

    decoded = ptr + FRONT_PIC_TABLE_SUB_KEY2 - FRONT_PIC_TABLE_ADD_KEY2
              + FRONT_PIC_TABLE_SUB_KEY1 - FRONT_PIC_TABLE_ADD_KEY1;
    if (IsRomPtr(decoded))
        return decoded;

    decoded = ptr + FRONT_PIC_TABLE_LEGACY_KEY;
    if (IsRomPtr(decoded))
        return decoded;

    return ptr;
}

const struct CompressedSpriteSheet *ResolveFrontPicTable(void)
{
    return (const struct CompressedSpriteSheet *)DecodeFrontPicTablePtrValue((u32)GetEncodedFrontPicTable());
}

const struct CompressedSpriteSheet *ResolveFrontPicSheet(u16 species)
{
    return &ResolveFrontPicTable()[species];
}

const struct CompressedSpriteSheet *DecodeFrontPicSheetPointer(const struct CompressedSpriteSheet *src)
{
    u32 ptr = (u32)src;
    u32 encodedBase = (u32)GetEncodedFrontPicTable();
    u32 encodedEnd = encodedBase + (NUM_SPECIES * sizeof(struct CompressedSpriteSheet));
    u32 decodedBase = (u32)ResolveFrontPicTable();
    u32 legacyBase = decodedBase - FRONT_PIC_TABLE_LEGACY_KEY;
    u32 legacyEnd = legacyBase + (NUM_SPECIES * sizeof(struct CompressedSpriteSheet));

    if (ptr >= encodedBase && ptr < encodedEnd)
        return (const struct CompressedSpriteSheet *)DecodeFrontPicTablePtrValue(ptr);

    if (ptr >= legacyBase && ptr < legacyEnd)
        return (const struct CompressedSpriteSheet *)(ptr + FRONT_PIC_TABLE_LEGACY_KEY);

    return src;
}

void DecompressPicFromTable_Resolved(const struct CompressedSpriteSheet *src, void *buffer, s32 species)
{
    src = DecodeFrontPicSheetPointer(src);

    if (species > NUM_SPECIES)
        LZ77UnCompWram(ResolveFrontPicTable()[0].data, buffer);
    else
        LZ77UnCompWram(src->data, buffer);

    DuplicateDeoxysTiles(buffer, species);
}

void DecompressPicFromTableDontHandleDeoxys_Resolved(const struct CompressedSpriteSheet *src,
                                                     void *buffer, s32 species)
{
    src = DecodeFrontPicSheetPointer(src);

    if (species > NUM_SPECIES)
        LZ77UnCompWram(ResolveFrontPicTable()[0].data, buffer);
    else
        LZ77UnCompWram(src->data, buffer);
}

void HandleLoadSpecialPokePic_Resolved(const struct CompressedSpriteSheet *src, void *dest,
                                       s32 species, u32 personality)
{
    src = DecodeFrontPicSheetPointer(src);
    LoadSpecialPokePic(src, dest, species, personality, src == ResolveFrontPicSheet(species));
}

void HandleLoadSpecialPokePicDontHandleDeoxys_Resolved(const struct CompressedSpriteSheet *src,
                                                       void *dest, s32 species, u32 personality)
{
    src = DecodeFrontPicSheetPointer(src);
    LoadSpecialPokePic(src, dest, species, personality, src == ResolveFrontPicSheet(species));
}
