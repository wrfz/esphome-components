import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import *
from esphome.core import Lambda
from esphome.cpp_types import std_ns
from esphome.components import sensor, binary_sensor, text_sensor, uart
from esphome.components.uart import UARTComponent
from enum import Enum

CODEOWNERS = ["@wrfz"]
AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]
DEPENDENCIES = ["uart"]

daikin_rotex_uart_ns = cg.esphome_ns.namespace("daikin_rotex_uart")
DaikinRotexUARTComponent = daikin_rotex_uart_ns.class_(
    "DaikinRotexUARTComponent", cg.Component, uart.UARTDevice
)

UartSensor = daikin_rotex_uart_ns.class_("UartSensor", sensor.Sensor)
UartTextSensor = daikin_rotex_uart_ns.class_("UartTextSensor", text_sensor.TextSensor)
UartBinarySensor = daikin_rotex_uart_ns.class_("UartBinarySensor", binary_sensor.BinarySensor)

EndianLittle = daikin_rotex_uart_ns.enum('TEntity::Endian::Little')
EndianBig = daikin_rotex_uart_ns.enum('TEntity::Endian::Big')

class Endian(Enum):
    LITTLE = 1
    BIG = 2

UNIT_REVOLUTIONS_PER_SECOND = "rps"
UNIT_PRESSURE_BAR = "bar"

RRLQ006CAV3 = "RRLQ006CAV3"
RRLQ008CAV3 = "RRLQ008CAV3"
RRLQ011CAW1 = "RRLQ011CAW1"
ERLQ016CAW1 = "ERLQ016CAW1"
ERGA08DAV3 = "ERGA08DAV3"
RRGA08DAV3 = "RRGA08DAV3"

OUTDOOR_UNIT = {
    RRLQ006CAV3: 1,
    RRLQ008CAV3: 2,
    RRLQ011CAW1: 3,
    ERLQ016CAW1: 4,
    ERGA08DAV3: 5,
    RRGA08DAV3: 6
}

current_outdoor_unit = None

fan_divider = {
    RRLQ006CAV3: 0.1,
    RRLQ008CAV3: 0.1,
    RRLQ011CAW1: 0.1 * 0.1,
    ERLQ016CAW1: 0.1 * 0.1,
    ERGA08DAV3: 0.1 * 0.1,
    RRGA08DAV3: 0.1 * 0.1,
}

def get_fan_divider():
    return fan_divider.get(current_outdoor_unit)

sensor_configuration = [
    {
        "type": "binary_sensor",
        "name": "press_eq", # Druckausgleich
        "registryID": 0x10,
        "offset": 1,
        "handle_lambda": """
            return (data[0] & 0x04) > 0;
        """,
        "icon": "mdi:car-brake-low-pressure"
    },
    {
        "type": "binary_sensor",
        "name": "defrost_operation", # Abtauvorgang
        "registryID": 0x10,
        "offset": 1,
        "handle_lambda": """
            return (data[0] & 0x10) > 0;
        """,
        "icon": "mdi:sun-snowflake-variant"
    },
    {
        "type": "sensor",
        "name": "tgt_cond_temp",
        "registryID": 0x10,
        "offset": 6,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT,
        "handle_lambda": """
            const int16_t value = static_cast<int16_t>(data[1] << 8 | data[0]);
            return value / 256.0 * 10.0 * 10.0;
        """
    },
    {
        "type": "sensor",
        "name": "outdoor_temp",
        "registryID": 0x20,
        "offset": 0,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "temp_after_compr",
        "registryID": 0x20,
        "offset": 4,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "exch_temp",
        "registryID": 0x20,
        "offset": 8,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "liq_pressure",
        "registryID": 0x20,
        "offset": 14,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_PRESSURE,
        #"unit_of_measurement": UNIT_BAR,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "t_liq2",
        "registryID": 0x20,
        "offset": 14,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT,
        "handle_lambda": """
            const double pressureBar = (data[0] + (data[1] << 8)) / 10.0;
            const double divider = 10.0;

            const std::vector<std::pair<double, double>> table = {
                {0.482,-65.0},
                {0.511,-64.0},
                {0.542,-63.0},
                {0.574,-62.0},
                {0.607,-61.0},
                {0.642,-60.0},
                {0.679,-59.0},
                {0.717,-58.0},
                {0.757,-57.0},
                {0.799,-56.0},
                {0.843,-55.0},
                {0.888,-54.0},
                {0.935,-53.0},
                {0.985,-52.0},
                {1.036,-51.0},
                {1.090,-50.0},
                {1.146,-49.0},
                {1.204,-48.0},
                {1.264,-47.0},
                {1.326,-46.0},
                {1.391,-45.0},
                {1.459,-44.0},
                {1.529,-43.0},
                {1.602,-42.0},
                {1.677,-41.0},
                {1.755,-40.0},
                {1.836,-39.0},
                {1.920,-38.0},
                {2.007,-37.0},
                {2.096,-36.0},
                {2.189,-35.0},
                {2.285,-34.0},
                {2.385,-33.0},
                {2.487,-32.0},
                {2.593,-31.0},
                {2.703,-30.0},
                {2.816,-29.0},
                {2.933,-28.0},
                {3.053,-27.0},
                {3.177,-26.0},
                {3.306,-25.0},
                {3.438,-24.0},
                {3.574,-23.0},
                {3.714,-22.0},
                {3.858,-21.0},
                {4.007,-20.0},
                {4.160,-19.0},
                {4.317,-18.0},
                {4.479,-17.0},
                {4.645,-16.0},
                {4.817,-15.0},
                {4.992,-14.0},
                {5.173,-13.0},
                {5.359,-12.0},
                {5.550,-11.0},
                {5.746,-10.0},
                {5.947,-9.0},
                {6.153,-8.0},
                {6.365,-7.0},
                {6.583,-6.0},
                {6.806,-5.0},
                {7.034,-4.0},
                {7.268,-3.0},
                {7.509,-2.0},
                {7.755,-1.0},
                {8.007,0.0},
                {8.265,1.0},
                {8.530,2.0},
                {8.801,3.0},
                {9.078,4.0},
                {9.362,5.0},
                {9.653,6.0},
                {9.950,7.0},
                {10.25,8.0},
                {10.57,9.0},
                {10.88,10.0},
                {11.21,11.0},
                {11.54,12.0},
                {11.88,13.0},
                {12.23,14.0},
                {12.58,15.0},
                {12.95,16.0},
                {13.32,17.0},
                {13.70,18.0},
                {14.08,19.0},
                {14.48,20.0},
                {14.88,21.0},
                {15.29,22.0},
                {15.71,23.0},
                {16.14,24.0},
                {16.57,25.0},
                {17.02,26.0},
                {17.48,27.0},
                {17.94,28.0},
                {18.41,29.0},
                {18.89,30.0},
                {19.39,31.0},
                {19.89,32.0},
                {20.40,33.0},
                {20.92,34.0},
                {21.45,35.0},
                {21.99,36.0},
                {22.54,37.0},
                {23.10,38.0},
                {23.67,39.0},
                {24.26,40.0},
                {24.85,41.0},
                {25.45,42.0},
                {26.07,43.0},
                {26.70,44.0},
                {27.34,45.0},
                {27.99,46.0},
                {28.65,47.0},
                {29.32,48.0},
                {30.01,49.0},
                {30.71,50.0},
                {31.42,51.0},
                {32.14,52.0},
                {32.88,53.0},
                {33.63,54.0},
                {34.39,55.0},
                {35.17,56.0},
                {35.96,57.0},
                {36.76,58.0},
                {37.58,59.0},
                {38.42,60.0},
                {39.27,61.0},
                {40.13,62.0},
                {41.01,63.0},
                {41.91,64.0},
                {42.82,65.0},
                {43.75,66.0},
                {44.70,67.0},
                {45.67,68.0},
                {46.65,69.0},
                {47.65,70.0},
                {48.67,71.0},
            };

            if (pressureBar < table.front().first || pressureBar > table.back().first) {
                return -1; // Druck außerhalb des gültigen Bereichs!
            }

            for (size_t i = 1; i < table.size(); ++i) {
                if (pressureBar <= table[i].first) {
                    double x1 = table[i - 1].first;
                    double y1 = table[i - 1].second;
                    double x2 = table[i].first;
                    double y2 = table[i].second;

                    return (y1 + (pressureBar - x1) * (y2 - y1) / (x2 - x1)) * divider;
                }
            }

            return -1;
        """
    },
    {
        "type": "sensor",
        "name": "inv_prim_current",
        "registryID": 0x21,
        "offset": 0,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_CURRENT,
        "unit_of_measurement": UNIT_AMPERE,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "inv_sec_current",
        "registryID": 0x21,
        "offset": 2,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_CURRENT,
        "unit_of_measurement": UNIT_AMPERE,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "inv_freq",
        "registryID": 0x30,
        "offset": 0,
        "signed": False,
        "dataSize": 1,
        "divider": 1.0 / 60.0,
        "unit_of_measurement": UNIT_REVOLUTIONS_PER_MINUTE,
        "state_class": STATE_CLASS_MEASUREMENT,
        "icon": "mdi:fan"
    },
    {
        "type": "sensor",
        "name": "fan_speed",
        "registryID": 0x30,
        "offset": 1,
        "signed": False,
        "dataSize": 1,
        "divider": get_fan_divider,
        "unit_of_measurement": UNIT_REVOLUTIONS_PER_MINUTE,
        "state_class": STATE_CLASS_MEASUREMENT,
        "icon": "mdi:fan"
    },
    {
        "type": "sensor",
        "name": "exv",
        "registryID": 0x30,
        "offset": 3,
        "signed": False,
        "dataSize": 2,
        "divider": 500.0 / 100.0, # 500pls = 100%
        "endian": Endian.LITTLE,
        "unit_of_measurement": UNIT_PERCENT,
        "state_class": STATE_CLASS_MEASUREMENT,
        "icon": "mdi:pipe-valve"
    },
    {
        "type": "binary_sensor",
        "name": "4_way_valve",
        "registryID": 0x30,
        "offset": 11,
        "handle_lambda": """
            return (data[0] & 0x80) > 0;
        """,
        "icon": "mdi:numeric-4-circle-outline"
    },
    {
        "type": "text_sensor",
        "name": "mode_of_operating", # Betriebsart
        "registryID": 0x60,
        "offset": 2,
        "signed": False,
        "dataSize": 1,
        "icon": "mdi:sun-snowflake-variant",
        "map": {
            0x00: "Standby",
            0x01: "Heizen",
            0x02: "Kühlen",
            0x03: "???",
            0x04: "Warmwasserbereitung",
            0x05: "Heizen + Warmwasser",
            0x06: "Kühlen + Warmwasser"
        },
        "handle_lambda": """
            return data[0] >> 4;
        """
    },
    {
        "type": "binary_sensor",
        "name": "buh1",
        "registryID": 0x60,
        "offset": 12,
        "handle_lambda": """
            return (data[0] & 0x10) > 0;
        """,
        "icon": "mdi:heat-wave"
    },
    {
        "type": "binary_sensor",
        "name": "buh2",
        "registryID": 0x60,
        "offset": 12,
        "handle_lambda": """
            return (data[0] & 0x08) > 0;
        """,
        "icon": "mdi:heat-wave"
    },
    {
        "type": "binary_sensor",
        "name": "buh_bsh",
        "registryID": 0x60,
        "offset": 12,
        "handle_lambda": """
            return (data[0] & 0x20) > 0;
        """,
        "icon": "mdi:heat-wave"
    },
    {
        "type": "sensor",
        "name": "tv",
        "registryID": 0x61,
        "offset": 2,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "tv_bh",
        "registryID": 0x61,
        "offset": 4,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "t_liq",
        "registryID": 0x61,
        "offset": 6,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "tr",
        "registryID": 0x61,
        "offset": 8,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "dhw_temp",
        "registryID": 0x61,
        "offset": 10,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT
    }

    # 0x62 Not supported by HPSU Compact 2013 + RRLQ006CAV3
]

def validate_setoutdoor_unit(value):
    global current_outdoor_unit
    current_outdoor_unit = value
    return value

CONF_ENTITIES = "entities"
CONF_OUTDOR_UNIT = "outdoor_unit"

entity_schemas = {}
for sensor_conf in sensor_configuration:
    name = sensor_conf.get("name")

    match sensor_conf.get("type"):
        case "sensor":
            entity_schemas.update({
                cv.Optional(name): sensor.sensor_schema(
                    UartSensor,
                    device_class=(sensor_conf.get("device_class", sensor._UNDEF)),
                    unit_of_measurement=(sensor_conf.get("unit_of_measurement", sensor._UNDEF)),
                    accuracy_decimals=(sensor_conf.get("accuracy_decimals", sensor._UNDEF)),
                    state_class=(sensor_conf.get("state_class", sensor._UNDEF)),
                    icon=(sensor_conf.get("icon", sensor._UNDEF))
                ),
            })
        case "binary_sensor":
            entity_schemas.update({
                cv.Optional(name): binary_sensor.binary_sensor_schema(
                    UartBinarySensor,
                    icon=sensor_conf.get("icon", binary_sensor._UNDEF)
                )
            })
        case "text_sensor":
            entity_schemas.update({
                cv.Optional(name): text_sensor.text_sensor_schema(
                    UartTextSensor,
                    icon=sensor_conf.get("icon", text_sensor._UNDEF)
                )
            })

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DaikinRotexUARTComponent),
        cv.Required(CONF_UART_ID): cv.use_id(UARTComponent),
        cv.Required(CONF_OUTDOR_UNIT): cv.ensure_list(cv.enum(OUTDOOR_UNIT), validate_setoutdoor_unit),
        cv.Required(CONF_ENTITIES): cv.Schema(
            entity_schemas
        )
    }
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "daikin_rotex_uart",
    baud_rate=9600,
    require_tx=True,
    require_rx=True,
    data_bits=8,
    #parity=None,
    stop_bits=1,
)

async def to_code(config):
    u8_ptr = std_ns.class_("uint8_t*")
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if entities := config.get(CONF_ENTITIES):
        for sens_conf in sensor_configuration:
            if yaml_sensor_conf := entities.get(sens_conf.get("name")):
                entity = None

                divider = sens_conf.get("divider", 1.0)
                mapping = sens_conf.get("map", {})
                str_map = "|".join([f"0x{int(key * divider) & 0xFFFF :02X}:{value}" for key, value in mapping.items()])

                match sens_conf.get("type"):
                    case "sensor":
                        entity = await sensor.new_sensor(yaml_sensor_conf)
                    case "binary_sensor":
                        entity = await binary_sensor.new_binary_sensor(yaml_sensor_conf)
                    case "text_sensor":
                        entity = await text_sensor.new_text_sensor(yaml_sensor_conf)
                        cg.add(entity.set_map(str_map))

                async def handle_lambda():
                    lamb = sens_conf.get("handle_lambda", "return 0;")
                    return await cg.process_lambda(
                        Lambda(lamb),
                        [(u8_ptr, "data")],
                        return_type=cg.uint16,
                    )

                if callable(divider):
                    divider = divider()
                cg.add(entity.set_entity([
                    entity,
                    sens_conf.get("name"),
                    sens_conf.get("registryID"),
                    sens_conf.get("offset"),
                    sens_conf.get("signed", True),
                    sens_conf.get("dataSize", 0),
                    EndianLittle if sens_conf.get("endian") == Endian.LITTLE else EndianBig,
                    divider,
                    sens_conf.get("accuracy_decimals", 0),
                    await handle_lambda(),
                    "handle_lambda" in sens_conf
                ]))
                cg.add(var.add_entity(entity))