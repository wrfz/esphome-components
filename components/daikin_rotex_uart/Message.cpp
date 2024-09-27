#include "esphome/components/daikin_rotex_uart/Message.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
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

void TMessage::convert(uint8_t* data) {
    std::string str;
    double dblData = std::numeric_limits<double>::quiet_NaN();

    switch (m_convId)
    {
    case 100:
        str = std::string(data, data + m_dataSize);
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
        convertTable200(data, str);
        return;
    case 203:
        convertTable203(data, str);
        return;
    case 211:
        if (data == 0)
        {
            str = "OFF";
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
        str = Utils::format("{0:%d}{1:%d}", num, num2);
        return;
    }

    case 300:
    case 301:
    case 302:
    case 303:
    case 304:
    case 305:
    case 306:
    case 307:
        convertTable300(data, m_convId, str);
        return;
    case 312:
        dblData = convertTable312(data);
        break;
    case 315:
        convertTable315(data, str);
        return;
    case 316:
        convertTable316(data, str);
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
        str = Utils::format("Conv %d not avail.", m_convId);
        return;
    }
    if (!std::isnan(dblData))
    {
        str = Utils::format("%g", dblData);
        ESP_LOGI("CONV", "name: %s, value: %s", m_pEntity->get_name().c_str(), str.c_str());
        if (sensor::Sensor* pSensor = dynamic_cast<sensor::Sensor*>(m_pEntity)) {
            pSensor->publish_state(dblData);
        }
    }
}

void TMessage::convertTable300(unsigned char *data, int tableID, std::string& str)
{
    //Serial.printf("Bin Conv %02x with tableID %d \n", data[0], tableID);
    char b = 1;
    b = (char)(b << tableID % 10);
    if ((data[0] & b) > 0)
    {
        str = "ON";
    }
    else
    {
        str = "OFF";
    }
    return;
}

void TMessage::convertTable203(unsigned char *data, std::string& str)
{
    switch (data[0])
    {
    case 0:
        str = "Normal";
        break;
    case 1:
        str = "Error";
        break;
    case 2:
        str = "Warning";
        break;
    case 3:
        str = "Caution";
        break;
    default:
        str = "-";
        ;
    }
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

void TMessage::convertTable315(unsigned char *data, std::string& str)
{
    char b = 240 & data[0];
    b = (char)(b >> 4);
    switch (b)
    {
    case 0:
        str = "Stop";
        break;
    case 1:
        str = "Heating";
        break;
    case 2:
        str = "Cooling";
        break;
    case 3:
        str = "??";
        break;
    case 4:
        str = "DHW";
        break;
    case 5:
        str = "Heating + DHW";
        break;
    case 6:
        str = "Cooling + DHW";
        break;
    default:
        str = "-";
    }
}

void TMessage::convertTable316(unsigned char *data, std::string& str)
{
    char byte = (data[0] >> 4) & 0xF;
    switch (byte)
    {
    case 0:
        str = "H/P only";
        break;
    case 1:
        str = "Hybrid";
        break;
    case 2:
        str = "Boiler only";
        break;
    default:
        str = "Unknown";
    }
}

void TMessage::convertTable200(unsigned char *data, std::string& str)
{
    if (data[0] == 0)
    {
        str = "OFF";
    }
    else
    {
        str = "ON";
    }
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