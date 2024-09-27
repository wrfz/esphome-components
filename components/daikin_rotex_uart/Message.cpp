#include "esphome/components/daikin_rotex_uart/Message.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/log.h"

#include <limits>
#include <cstring>
#include <cmath>

namespace esphome {
namespace daikin_rotex_uart {

TMessage::TMessage(
    std::shared_ptr<TRequest> pRequest,
    EntityBase* pEntity,
    uint8_t registryID,
    uint8_t offset,
    int convid,
    int dataSize,
    int dataType
)
: m_pRequest(pRequest)
, m_pEntity(pEntity)
, m_convId(convid)
, m_offset(offset)
, m_registryID(registryID)
, m_dataSize(dataSize)
, m_dataType(dataType)
{
}

size_t                   /* O - Length of string */
strlcat(char *dst,       /* O - Destination string */
        const char *src, /* I - Source string */
        size_t size)     /* I - Size of destination string buffer */
{
    size_t srclen; /* Length of source string */
    size_t dstlen; /* Length of destination string */

    /*
  * Figure out how much room is left...
  */

    dstlen = std::strlen(dst);
    size -= dstlen + 1;

    if (!size)
        return (dstlen); /* No room, return immediately... */

    /*
  * Figure out how much room is needed...
  */

    srclen = std::strlen(src);

    /*
  * Copy the appropriate amount...
  */

    if (srclen > size)
        srclen = size;

    std::memcpy(dst + dstlen, src, srclen);
    dst[dstlen + srclen] = '\0';

    return (dstlen + srclen);
}

void TMessage::convert(uint8_t* data) {
    m_asString[0] = {0};
    double dblData = std::numeric_limits<double>::quiet_NaN();

    switch (m_convId)
    {
    case 100:
        strlcat(m_asString, (char*)data, m_dataSize);
        return;
    case 101:
        dblData = getSignedValue(data, m_dataSize, Endian::Little);
        break;
    case 102:
        dblData = getSignedValue(data, m_dataSize, Endian::Big);
        break;
    case 103:
        dblData = getSignedValue(data, m_dataSize, Endian::Little) / 256.0;
        break;
    case 104:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) / 256.0;
        break;
    case 105:
        dblData = getSignedValue(data, m_dataSize, Endian::Little) * 0.1;
        break;
    case 106:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) * 0.1;
        break;
    case 107:
        dblData = getSignedValue(data, m_dataSize, Endian::Little) * 0.1;
        if (dblData == -3276.8)
        {
            strcat(m_asString, "---");
            return;
        }
        break;
    case 108:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) * 0.1;
        if (dblData == -3276.8)
        {
            strcat(m_asString, "---");
            return;
        }
        break;
    case 109:
        dblData = getSignedValue(data, m_dataSize, Endian::Little) / 256.0 * 2.0;
        break;
    case 110:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) / 256.0 * 2.0;
        break;
    case 111:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) * 0.5;
        break;
    case 112:
        dblData = (getSignedValue(data, m_dataSize, Endian::Big) - 64) * 0.5;
        break;
    case 113:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) * 0.25;
        break;
    case 114:
    {
        if (data[0] == 0 && data[1] == 128)
        {
            strcat(m_asString, "---");
            return;
        }
        uint16_t num2 = (uint16_t)((int)data[1] * 256);
        num2 |= (uint16_t)data[0];
        if ((data[1] & 128) != 0)
        {
            num2 = ~(num2 - 1);
        }
        dblData = (num2 & 0xFF00) / 256.0;
        dblData += (num2 & 255) / 256.0;
        dblData *= 10.0;
        if ((data[1] & 128) != 0)
        {
            dblData *= -1.0;
        }
        break;
    }
    case 115:
        dblData = getSignedValue(data, m_dataSize, Endian::Little) / 2560.0;
        break;
    case 116:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) / 2560.0;
        break;
    case 117:
        dblData = getSignedValue(data, m_dataSize, Endian::Little) * 0.01;
        break;
    case 118:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) * 0.01;
        break;
    case 119:
    {
        if (data[0] == 0 && data[1] == 128)
        {
            strcat(m_asString, "---");
            return;
        }
        uint16_t num3 = (uint16_t)((int)data[1] * 256);
        num3 |= (uint16_t)(data[0] & 127);
        dblData = (num3 & 65280) / 256.0;
        dblData += (num3 & 255) / 256.0;
        break;
    }
    case 151:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Little);
        break;
    case 152:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Big);
        break;
    case 153:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Little) / 256.0;
        break;
    case 154:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Big) / 256.0;
        break;
    case 155:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Little) * 0.1;
        break;
    case 156:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Big) * 0.1;
        break;
    case 157:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Little) / 256.0 * 2.0;
        break;
    case 158:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Big) / 256.0 * 2.0;
        break;
    case 161:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Big) * 0.5;
        break;
    case 162:
        dblData = (getUnsignedValue(data, m_dataSize, Endian::Big) - 64) * 0.5;
        break;
    case 163:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Big) * 0.25;
        break;
    case 164:
        dblData = getUnsignedValue(data, m_dataSize, Endian::Big) * 5;
        break;
    case 165:
        dblData = (getUnsignedValue(data, m_dataSize, Endian::Little) & 0x3FFF);
        break;
    case 200:
        convertTable200(data, m_asString);
        return;
    case 203:
        convertTable203(data, m_asString);
        return;
    case 204:
        convertTable204(data, m_asString);
        return;
    case 211:
        if (data == 0)
        {
            strcat(m_asString, "OFF");
            return;
        }
        else
        {
            dblData = (uint)(double)data[0];
            break;
        }

    case 215:
    case 216:
    {
        int num = data[0] >> 4;
        int num2 = (int)(data[0] & 0xF);
        sprintf(m_asString,"{0:%d}{1:%d}", num, num2);
        return;
    }

    case 201:
    case 217:
        convertTable217(data, m_asString);
        return;
    case 300:
    case 301:
    case 302:
    case 303:
    case 304:
    case 305:
    case 306:
    case 307:
        convertTable300(data, m_convId, m_asString);
        return;
    case 312:
        dblData = convertTable312(data);
        break;
    case 315:
        convertTable315(data, m_asString);
        return;
    case 316:
        convertTable316(data, m_asString);
        return;

    // pressure to temp
    case 401:
        dblData = getSignedValue(data, m_dataSize, Endian::Little);
        dblData = convertPress2Temp(dblData);
        break;
    case 402:
        dblData = getSignedValue(data, m_dataSize, Endian::Big);
        dblData = convertPress2Temp(dblData);
        break;
    case 403:
        dblData = getSignedValue(data, m_dataSize, Endian::Little) / 256.0;
        dblData = convertPress2Temp(dblData);
        break;
    case 404:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) / 256.0;
        dblData = convertPress2Temp(dblData);
        break;
    case 405:
        dblData = getSignedValue(data, m_dataSize, Endian::Little) * 0.1;
        dblData = convertPress2Temp(dblData);
        break;
    case 406:
        dblData = getSignedValue(data, m_dataSize, Endian::Big) * 0.1;
        dblData = convertPress2Temp(dblData);
        break;

    default:
        // conversion is not available
        sprintf(m_asString, "Conv %d not avail.", m_convId);
        return;
    }
    if (!std::isnan(dblData))
    {
        sprintf(m_asString, "%g", dblData);
        ESP_LOGI("CONV", "name: %s, value: %s", m_pEntity->get_name().c_str(), m_asString);
        if (sensor::Sensor* pSensor = dynamic_cast<sensor::Sensor*>(m_pEntity)) {
            pSensor->publish_state(dblData);
        }
    }
}

void TMessage::convertTable300(unsigned char *data, int tableID, char *ret)
{
    //Serial.printf("Bin Conv %02x with tableID %d \n", data[0], tableID);
    char b = 1;
    b = (char)(b << tableID % 10);
    if ((data[0] & b) > 0)
    {
        strcat(ret, "ON");
    }
    else
    {
        strcat(ret, "OFF");
    }
    return;
}

void TMessage::convertTable203(unsigned char *data, char *ret)
{
    switch (data[0])
    {
    case 0:
        strcat(ret, "Normal");
        break;
    case 1:
        strcat(ret, "Error");
        break;
    case 2:
        strcat(ret, "Warning");
        break;
    case 3:
        strcat(ret, "Caution");
        break;
    default:
        strcat(ret, "-");
        ;
    }
}

void TMessage::convertTable204(unsigned char *data, char *ret)
{
    char array[] = " ACEHFJLPU987654";
    char array2[] = "0123456789AHCJEF";
    int num = data[0] >> 4 & 0xF;
    int num2 = (int)(data[0] & 0xF);
    ret[0] = array[num];
    ret[1] = array2[num2];
    ret[2] = 0;
}

double TMessage::convertTable312(unsigned char *data)
{
    double dblData = ((unsigned char) (7 & data[0] >> 4) + (unsigned char) (15U & data[0])) / 16.0;
    if ((128 & data[0]) > 0)
    {
        dblData *= -1.0;
    }
    return dblData;
}

void TMessage::convertTable315(unsigned char *data, char *ret)
{
    char b = 240 & data[0];
    b = (char)(b >> 4);
    switch (b)
    {
    case 0:
        strcat(ret, "Stop");
        break;
    case 1:
        strcat(ret, "Heating");
        break;
    case 2:
        strcat(ret, "Cooling");
        break;
    case 3:
        strcat(ret, "??");
        break;
    case 4:
        strcat(ret, "DHW");
        break;
    case 5:
        strcat(ret, "Heating + DHW");
        break;
    case 6:
        strcat(ret, "Cooling + DHW");
        break;
    default:
        strcat(ret, "-");
    }
}

void TMessage::convertTable316(unsigned char *data, char *ret)
{
    char b = 240 & data[0];
    b = (char)(b >> 4);
    switch (b)
    {
    case 0:
        strcat(ret, "H/P only");
        break;
    case 1:
        strcat(ret, "Hybrid");
        break;
    case 2:
        strcat(ret, "Boiler only");
        break;
    default:
        strcat(ret, "Unknown");
    }
}

void TMessage::convertTable200(unsigned char *data, char *ret)
{
    if (data[0] == 0)
    {
        strcat(ret, "OFF");
    }
    else
    {
        strcat(ret, "ON");
    }
}

// 201
void TMessage::convertTable217(unsigned char *data, char *ret)
{
    char r217[][30] = {"Fan Only",
                        "Heating",
                        "Cooling",
                        "Auto",
                        "Ventilation",
                        "Auto Cool",
                        "Auto Heat",
                        "Dry",
                        "Aux.",
                        "Cooling Storage",
                        "Heating Storage",
                        "UseStrdThrm(cl)1",
                        "UseStrdThrm(cl)2",
                        "UseStrdThrm(cl)3",
                        "UseStrdThrm(cl)4",
                        "UseStrdThrm(ht)1",
                        "UseStrdThrm(ht)2",
                        "UseStrdThrm(ht)3",
                        "UseStrdThrm(ht)4"};
    sprintf(ret, r217[(int)data[0]]);
}

uint16_t TMessage::getUnsignedValue(unsigned char *data, int dataSize, Endian endian) {
    uint16_t result;
    if (dataSize == 1)
    {
        result = (uint16_t)data[0];
    }
    else if (endian == Endian::Little)
    {
        result = ((uint16_t)(data[1] << 8) | (uint16_t)data[0]);
    }
    else // Big Endian
    {
        result = ((uint16_t)(data[0] << 8) | (uint16_t)data[1]);
    }
    return result;
}

short TMessage::getSignedValue(unsigned char *data, int datasize, Endian endian) {
    uint16_t num = getUnsignedValue(data, datasize, endian);
    short result = (short)num;
    if ((num & 0x8000) != 0)
    {
        num = ~num;
        num += 1;
        result = (short)((int)num * -1);
    }
    return result;
}

double TMessage::convertPress2Temp(double data){//assuming R32 gaz
    double num = -2.6989493795556E-07 * data * data * data * data * data * data;
    double num2 = 4.26383417104661E-05 * data * data * data * data * data;
    double num3 = -0.00262978346547749 * data * data * data * data;
    double num4 = 0.0805858127503585 * data * data * data;
    double num5 = -1.31924457284073 * data * data;
    double num6 = 13.4157368435437 * data;
    double num7 = -51.1813342993155;
    return num + num2 + num3 + num4 + num5 + num6 + num7;
}

} // namespace daikin_rotex_uart
} // namespace esphome