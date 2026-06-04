#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ipc_shareMem.h"

extern volatile ipc_data_t gSharedMem;
extern uint8_t localOut[MAX_OUTPUT_SIZE_BYTES];
extern uint8_t localIn[MAX_INPUT_SIZE_BYTES];

// array size is 300
const unsigned char gsp_favicon_ico[]  = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xdb, 0x6d, 0x24, 0x20, 0x20, 0x24, 0x49, 0x92, 0xdb, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xb6, 0x49, 0x24, 0x24, 0x92, 0x92, 0x6d, 0x24, 0x24, 0x49, 0xdb, 0xff, 0xff, 
  0xff, 0xdb, 0x49, 0x24, 0x6d, 0xff, 0xff, 0xff, 0xff, 0xff, 0x49, 0x24, 0x6d, 0xff, 0xff, 
  0xff, 0xb6, 0x24, 0x6d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x49, 0x24, 0xdb, 0xff, 
  0xff, 0x8e, 0x20, 0xb6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x92, 0x24, 0xb6, 0xff, 
  0xff, 0x6d, 0x20, 0xb6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x92, 0x24, 0xb6, 0xff, 
  0xff, 0xb6, 0x20, 0x92, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x49, 0x24, 0xb6, 0xff, 
  0xff, 0xdb, 0x49, 0x24, 0xb6, 0xff, 0xff, 0xff, 0xff, 0xff, 0x92, 0x24, 0x49, 0xdb, 0xff, 
  0xff, 0xff, 0xb6, 0x24, 0x24, 0x6d, 0x92, 0xb6, 0x92, 0x49, 0x24, 0x49, 0xb6, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xdb, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x44, 0xdb, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xb6, 0x92, 0x92, 0x6d, 0x24, 0x24, 0xb6, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb6, 0x20, 0x49, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x49, 0xdb, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x20, 0x49, 0xdb, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x49, 0x24, 0x6d, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdb, 0x69, 0x24, 0x24, 0x69, 0xdb, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb6, 0x24, 0x49, 0xb6, 0xff, 0xff, 0xff, 0xff, 0xff
};
const unsigned int gsp_favicon_ico_len = sizeof(gsp_favicon_ico);

int generate_io_table(char *html_out, int max_size, IOCoupler_Device *dev)
{
    int offset = 0;
    int written = 0;
    int validCount = 0;

    if (!html_out || max_size <= 0 || !dev) return 0;

    html_out[0] = '\0';

    // ===== Header =====
    written = snprintf(html_out + offset, max_size - offset,
        "<br>"
        "<table id=\"iotable\" width=\"50%%\">"
        "<tbody>"
        "<tr>"
            "<th>ID</th>"
            "<th>IO Type</th>"
            "<th>Data Value</th>"
            "<th>State</th>"
            "<th>Last Error Type</th>"
            "<th>Last Error Code</th>"
        "</tr>"
    );

    if (written < 0 || written >= (max_size - offset)) return offset;
    offset += written;

    // ===== Rows =====
    for (int i = 0; i < dev->numberOfSlaves; i++)
    {
         char valueStr[64] = {0};
        IO_SlaveInfo *s = &dev->slaveInfo[i];
        // if (s->nodeId == 0) continue;

        validCount++;  // count valid IO

        IO_DigitalValues *dVal = NULL;

        /* ===== DIGITAL INPUT ===== */
        if (s->productCode == IO_DEVICE_TYPE_DI16)
        {
            if (s->offset_index + sizeof(IO_DigitalValues) <= sizeof(localIn))
            {
                dVal = (IO_DigitalValues *)&localIn[s->offset_index];
            }
        }
        
        /* ===== DIGITAL OUTPUT ===== */
        else if (s->productCode == IO_DEVICE_TYPE_DO16)
        {
            if (s->offset_index + sizeof(IO_DigitalValues) <= sizeof(localOut))
            {
                dVal = (IO_DigitalValues *)&localOut[s->offset_index];
            }
        }

        if (dVal != NULL)
        {
            for (int b = 15; b >= 0; b--)
            {
                valueStr[15 - b] =
                    ((dVal->d_all >> b) & 1U) ? '1' : '0';
            }

            valueStr[16] = '\0';
        }
        else
        {
            snprintf(valueStr, sizeof(valueStr), "N/A");
        }

        // ===== Append row =====
        written = snprintf(html_out + offset, max_size - offset,
            "<tr>"
                "<th>%d</th>"
                "<td>%d</td>"
                "<td>%s</td>"
                "<td>%d</td>"
                "<td>%d</td>"
                "<td>%d</td>"
            "</tr>",
            s->nodeId,
            s->productCode,
            valueStr,
            s->nodeState,
            s->last_error_type,
            s->last_error_code
        );

        if (written < 0 || written >= (max_size - offset))
        {
            break;
        }

        offset += written;
    }

    // ===== No IO case =====
    if (validCount == 0)
    {
        written = snprintf(html_out + offset, max_size - offset,
            "<tr>"
                "<td colspan=\"6\" style=\"text-align:center;\">No IO data available</td>"
            "</tr>"
        );

        if (written > 0 && written < (max_size - offset))
        {
            offset += written;
        }
    }

    // ===== Footer =====
    written = snprintf(html_out + offset, max_size - offset,
        "</tbody></table>"
    );

    if (written > 0 && written < (max_size - offset))
    {
        offset += written;
    }

    return offset;
}

/**
Output format:
[
  {
    "id":1,
    "productCode":2,
    "serialNumber":0000000,
    "state":1,
    "lastErrorType":0,
    "lastErrorCode":0,
    "value":5,
    "valueStr":"0000000000000101",
    "hwVer":"1.0.0",
    "fwVer":"2.1.4"
  },
  ...
]
 */
int generate_io_json(char *json_out, int max_size, IOCoupler_Device *dev)
{
    int offset = 0;
    int written = 0;

    if (!json_out || max_size <= 0 || !dev)
        return 0;

    json_out[0] = '\0';

    written = snprintf(json_out + offset,
                       max_size - offset,
                       "[");

    if (written < 0 || written >= (max_size - offset))
        return offset;

    offset += written;

    for (int i = 0; i < dev->numberOfSlaves; i++)
    {
        IO_SlaveInfo *s = &dev->slaveInfo[i];

        char valueStr[128] = "N/A";
        uint32_t value = 0;

        /* ==========================
         * Digital Input / Output
         * ========================== */
        if (s->productCode == IO_DEVICE_TYPE_DI16)
        {
            if (s->offset_index + sizeof(IO_DigitalValues) <= sizeof(localIn))
            {
                IO_DigitalValues *dVal =
                    (IO_DigitalValues *)&localIn[s->offset_index];

                value = dVal->d_all;

                for (int b = 15; b >= 0; b--)
                {
                    valueStr[15 - b] =
                        ((dVal->d_all >> b) & 1U) ? '1' : '0';
                }

                valueStr[16] = '\0';
            }
        }
        else if (s->productCode == IO_DEVICE_TYPE_DO16)
        {
            if (s->offset_index + sizeof(IO_DigitalValues) <= sizeof(localOut))
            {
                IO_DigitalValues *dVal =
                    (IO_DigitalValues *)&localOut[s->offset_index];

                value = dVal->d_all;

                for (int b = 15; b >= 0; b--)
                {
                    valueStr[15 - b] =
                        ((dVal->d_all >> b) & 1U) ? '1' : '0';
                }

                valueStr[16] = '\0';
            }
        }

        /* ==========================
         * Analog Input Voltage
         * ========================== */
        else if (   (s->productCode == IO_DEVICE_TYPE_AIV8) ||
                    (s->productCode == IO_DEVICE_TYPE_AIC8) )
        {
            if (s->offset_index + 16 <= sizeof(localIn))
            {
                IO_AnalogValues *aVal = (IO_AnalogValues *)&localIn[s->offset_index];

                snprintf(valueStr,
                         sizeof(valueStr),
                         "%d,%d,%d,%d,%d,%d,%d,%d",
                         aVal->a_ch1,
                         aVal->a_ch2,
                         aVal->a_ch3,
                         aVal->a_ch4,
                         aVal->a_ch5,
                         aVal->a_ch6,
                         aVal->a_ch7,
                         aVal->a_ch8);
            }
        }

        /* ==========================
         * Analog Output Voltage
         * ========================== */
        else if (   (s->productCode == IO_DEVICE_TYPE_AOV8) ||
                    (s->productCode == IO_DEVICE_TYPE_AOC8) )
        {
            if (s->offset_index + 16 <= sizeof(localOut))
            {
                IO_AnalogValues *aVal = (IO_AnalogValues *)&localOut[s->offset_index];

                snprintf(valueStr,
                         sizeof(valueStr),
                         "%d,%d,%d,%d,%d,%d,%d,%d",
                         aVal->a_ch1,
                         aVal->a_ch2,
                         aVal->a_ch3,
                         aVal->a_ch4,
                         aVal->a_ch5,
                         aVal->a_ch6,
                         aVal->a_ch7,
                         aVal->a_ch8);
            }
        }

        /* ==========================
         * RTD Modules
         * ========================== */
        else if ((s->productCode == IO_DEVICE_TYPE_RTDY) ||
                 (s->productCode == IO_DEVICE_TYPE_RTDB))
        {
            if (s->offset_index + 24 <= sizeof(localIn))
            {
                 // Handle RTD values if necessary
            }
        }

        /* UPDATED: Injected "hwVer" and "fwVer" fields into the format string template */
        written = snprintf(
            json_out + offset,
            max_size - offset,
            "%s{"
            "\"id\":%d,"
            "\"productCode\":%d,"
            "\"serialNumber\":%d,"
            "\"state\":%d,"
            "\"lastErrorType\":%d,"
            "\"lastErrorCode\":%d,"
            "\"value\":%u,"
            "\"valueStr\":\"%s\","
            "\"hwVer\":\"%s\","
            "\"fwVer\":\"%s\""
            "}",
            (i > 0) ? "," : "",
            s->nodeId,
            s->productCode,
            s->serialNumber,
            s->nodeState,
            s->last_error_type,
            s->last_error_code,
            value,
            valueStr,
            s->hw_version,
            s->fw_version);

        if (written < 0 || written >= (max_size - offset))
            break;

        offset += written;
    }

    written = snprintf(json_out + offset,
                       max_size - offset,
                       "]");

    if (written > 0 && written < (max_size - offset))
    {
        offset += written;
    }

    return offset;
}