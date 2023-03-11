#include "KT_BinIO.h"

uint32_t KT_BinIO::Read(char *pszInput)
{
    FILE *f;

    f = fopen(pszInput, "rb");
    if (f == NULL)
    {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    uint32_t lSize;
    lSize = ftell(f);
    rewind(f);
    if (fread(pReadBuff, 1, lSize, f) != lSize)
    {
        fclose(f);
        return 0;
    }
    fclose(f);

    return 1;
}

uint32_t KT_BinIO::Write(char *pszOutput)
{
    FILE *f;

    f = fopen(pszOutput, "wb");
    if (f == NULL)
    {
        return 0;
    }
    if (!fwrite(pWriteBuff, u32Size, 1, f))
    {
        fclose(f);
        return 0;
    }
    fclose(f);

    return 1;
}

uint32_t KT_BinIO::InitBuffer()
{

    pReadBuff = (uint8_t *)malloc(u32Size);
    if (pReadBuff == NULL)
    {
        return 0;
    }
    memset(pReadBuff, 0xFF, u32Size);
    pWriteBuff = (uint8_t *)malloc(u32Size);
    if (pWriteBuff == NULL)
    {
        return 0;
    }
    memset(pWriteBuff, 0xFF, u32Size);

    return 1;
}

void KT_BinIO::FreeBuffer()
{

    free(pReadBuff);
    free(pWriteBuff);
}
