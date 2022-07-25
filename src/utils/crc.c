/* *一口Linux *2021.6.21 *version: 1.0.0*/ 
#include "utils/crc.h" 
typedef enum{
    REF_4BIT = 4, 
    REF_5BIT = 5, 
    REF_6BIT = 6, 
    REF_7BIT = 7, 
    REF_8BIT = 8, 
    REF_16BIT = 16, 
    REF_32BIT = 32
} REFLECTED_MODE;
uint32_t reflected_data(uint32_t data, REFLECTED_MODE mode)
{
    data = ((data & 0xffff0000) >> 16) | ((data & 0x0000ffff) << 16);
    data = ((data & 0xff00ff00) >> 8) | ((data & 0x00ff00ff) << 8);
    data = ((data & 0xf0f0f0f0) >> 4) | ((data & 0x0f0f0f0f) << 4);
    data = ((data & 0xcccccccc) >> 2) | ((data & 0x33333333) << 2);
    data = ((data & 0xaaaaaaaa) >> 1) | ((data & 0x55555555) << 1);
    switch (mode)
    {
    case REF_32BIT:
        return data;
    case REF_16BIT:
        return (data >> 16) & 0xffff;
    case REF_8BIT:
        return (data >> 24) & 0xff;
    case REF_7BIT:
        return (data >> 25) & 0x7f;
    case REF_6BIT:
        return (data >> 26) & 0x7f;
    case REF_5BIT:
        return (data >> 27) & 0x1f;
    case REF_4BIT:
        return (data >> 28) & 0x0f;
    }
    return 0;
}
uint8_t check_crc4(uint8_t poly, uint8_t init, bool refin, bool refout, uint8_t xor_out, const uint8_t *buffer, uint32_t length)
{
    uint8_t i;
    uint8_t crc;
    if (refin == true)
    {
        crc = init;
        poly = reflected_data(poly, REF_4BIT);
        while (length--)
        {
            crc ^= *buffer++;
            for (i = 0; i < 8; i++)
            {
                if (crc & 0x01)
                {
                    crc >>= 1;
                    crc ^= poly;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
        return crc ^ xor_out;
    }
    else
    {
        crc = init << 4;
        poly <<= 4;
        while (length--)
        {
            crc ^= *buffer++;
            for (i = 0; i < 8; i++)
            {
                if (crc & 0x80)
                {
                    crc <<= 1;
                    crc ^= poly;
                }
                else
                {
                    crc <<= 1;
                }
            }
        }
        return (crc >> 4) ^ xor_out;
    }
}
uint8_t check_crc5(uint8_t poly, uint8_t init, bool refin, bool refout, uint8_t xor_out, const uint8_t *buffer, uint32_t length)
{
    uint8_t i;
    uint8_t crc;
    if (refin == true)
    {
        crc = init;
        poly = reflected_data(poly, REF_5BIT);
        while (length--)
        {
            crc ^= *buffer++;
            for (i = 0; i < 8; i++)
            {
                if (crc & 0x01)
                {
                    crc >>= 1;
                    crc ^= poly;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
        return crc ^ xor_out;
    }
    else
    {
        crc = init << 3;
        poly <<= 3;
        while (length--)
        {
            crc ^= *buffer++;
            for (i = 0; i < 8; i++)
            {
                if (crc & 0x80)
                {
                    crc <<= 1;
                    crc ^= poly;
                }
                else
                {
                    crc <<= 1;
                }
            }
        }
        return (crc >> 3) ^ xor_out;
    }
}
uint8_t check_crc6(uint8_t poly, uint8_t init, bool refin, bool refout, uint8_t xor_out, const uint8_t *buffer, uint32_t length)
{
    uint8_t i;
    uint8_t crc;
    if (refin == true)
    {
        crc = init;
        poly = reflected_data(poly, REF_6BIT);
        while (length--)
        {
            crc ^= *buffer++;
            for (i = 0; i < 8; i++)
            {
                if (crc & 0x01)
                {
                    crc >>= 1;
                    crc ^= poly;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
        return crc ^ xor_out;
    }
    else
    {
        crc = init << 2;
        poly <<= 2;
        while (length--)
        {
            crc ^= *buffer++;
            for (i = 0; i < 8; i++)
            {
                if (crc & 0x80)
                {
                    crc <<= 1;
                    crc ^= poly;
                }
                else
                {
                    crc <<= 1;
                }
            }
        }
        return (crc >> 2) ^ xor_out;
    }
}
uint8_t check_crc7(uint8_t poly, uint8_t init, bool refin, bool refout, uint8_t xor_out, const uint8_t *buffer, uint32_t length)
{
    uint8_t i;
    uint8_t crc;
    if (refin == true)
    {
        crc = init;
        poly = reflected_data(poly, REF_7BIT);
        while (length--)
        {
            crc ^= *buffer++;
            for (i = 0; i < 8; i++)
            {
                if (crc & 0x01)
                {
                    crc >>= 1;
                    crc ^= poly;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
        return crc ^ xor_out;
    }
    else
    {
        crc = init << 1;
        poly <<= 1;
        while (length--)
        {
            crc ^= *buffer++;
            for (i = 0; i < 8; i++)
            {
                if (crc & 0x80)
                {
                    crc <<= 1;
                    crc ^= poly;
                }
                else
                {
                    crc <<= 1;
                }
            }
        }
        return (crc >> 1) ^ xor_out;
    }
}
uint8_t check_crc8(uint8_t poly, uint8_t init, bool refin, bool refout, uint8_t xor_out, const uint8_t *buffer, uint32_t length)
{
    uint32_t i = 0;
    uint8_t crc = init;
    while (length--)
    {
        if (refin == true)
        {
            crc ^= reflected_data(*buffer++, REF_8BIT);
        }
        else
        {
            crc ^= *buffer++;
        }
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x80)
            {
                crc <<= 1;
                crc ^= poly;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    if (refout == true)
    {
        crc = reflected_data(crc, REF_8BIT);
    }
    return crc ^ xor_out;
}
uint16_t check_crc16(uint16_t poly, uint16_t init, bool refin, bool refout, uint16_t xor_out, const uint8_t *buffer, uint32_t length)
{
    uint32_t i = 0;
    uint16_t crc = init;
    while (length--)
    {
        if (refin == true)
        {
            crc ^= reflected_data(*buffer++, REF_8BIT) << 8;
        }
        else
        {
            crc ^= (*buffer++) << 8;
        }
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x8000)
            {
                crc <<= 1;
                crc ^= poly;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    if (refout == true)
    {
        crc = reflected_data(crc, REF_16BIT);
    }
    return crc ^ xor_out;
}
uint32_t check_crc32(uint32_t poly, uint32_t init, bool refin, bool refout, uint32_t xor_out, const uint8_t *buffer, uint32_t length)
{
    uint32_t i = 0;
    uint32_t crc = init;
    while (length--)
    {
        if (refin == true)
        {
            crc ^= reflected_data(*buffer++, REF_8BIT) << 24;
        }
        else
        {
            crc ^= (*buffer++) << 24;
        }
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x80000000)
            {
                crc <<= 1;
                crc ^= poly;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    if (refout == true)
    {
        crc = reflected_data(crc, REF_32BIT);
    }
    return crc ^ xor_out;
}
uint32_t crc_check(crc_type_t *crc_type, const uint8_t *buffer, uint32_t length)
{
    switch (crc_type->width)
    {
    case 4:
        return check_crc4(crc_type->poly, crc_type->init, crc_type->refin, crc_type->refout, crc_type->xor_out, buffer, length);
    case 5:
        return check_crc5(crc_type->poly, crc_type->init, crc_type->refin, crc_type->refout, crc_type->xor_out, buffer, length);
    case 6:
        return check_crc6(crc_type->poly, crc_type->init, crc_type->refin, crc_type->refout, crc_type->xor_out, buffer, length);
    case 7:
        return check_crc7(crc_type->poly, crc_type->init, crc_type->refin, crc_type->refout, crc_type->xor_out, buffer, length);
    case 8:
        return check_crc8(crc_type->poly, crc_type->init, crc_type->refin, crc_type->refout, crc_type->xor_out, buffer, length);
    case 16:
        return check_crc16(crc_type->poly, crc_type->init, crc_type->refin, crc_type->refout, crc_type->xor_out, buffer, length);
    case 32:
        return check_crc32(crc_type->poly, crc_type->init, crc_type->refin, crc_type->refout, crc_type->xor_out, buffer, length);
    }
    return 0;
}