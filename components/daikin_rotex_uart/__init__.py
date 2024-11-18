import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import *
from esphome.core import Lambda
from esphome.cpp_types import std_ns
from esphome.components import sensor, binary_sensor, uart
from esphome.components.uart import UARTComponent
from enum import Enum

CODEOWNERS = ["@wrfz"]
AUTO_LOAD = ["binary_sensor", "sensor"]
DEPENDENCIES = ["uart"]

daikin_rotex_uart_ns = cg.esphome_ns.namespace("daikin_rotex_uart")
DaikinRotexUARTComponent = daikin_rotex_uart_ns.class_(
    "DaikinRotexUARTComponent", cg.Component, uart.UARTDevice
)
EndianLittle = daikin_rotex_uart_ns.enum('TMessage::Endian::Little')
EndianBig = daikin_rotex_uart_ns.enum('TMessage::Endian::Big')

class Endian(Enum):
    LITTLE = 1
    BIG = 2

UNIT_REVOLUTIONS_PER_SECOND = "rps"
UNIT_PRESSURE_BAR = "bar"

RRLQ008CAV3 = "RRLQ008CAV3"
RRLQ011CAW1 = "RRLQ011CAW1"

OUTDOOR_UNIT = {
    RRLQ008CAV3: 1,
    RRLQ011CAW1: 2,
}

current_outdoor_unit = None

fan_divider = {
    RRLQ008CAV3: 0.1,
    RRLQ011CAW1: 0.1 * 0.1
}

def get_fan_divider():
    return fan_divider.get(current_outdoor_unit)

sensor_configuration = [
    {
        "type": "binary_sensor",
        "name": "pressure_equalization", # Druckausgleich
        "registryID": 0x10,
        "offset": 1,
        "handle_lambda": """
            return (data[0] & 0x04) > 0;
        """,
        "icon": "mdi:car-brake-low-pressure"
    },
    {
        "type": "sensor",
        "name": "target_liquefaction_pressure",
        "registryID": 0x10,
        "offset": 6,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 26.0 + (1.0 / 24.0),
        "unit_of_measurement": UNIT_PRESSURE_BAR,
        "accuracy_decimals": 1,
        "state_class": STATE_CLASS_MEASUREMENT,
        "icon": "mdi:arrow-collapse-all"
    },
    {
        "type": "sensor",
        "name": "outdoor_air_temp_r1t",
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
        "name": "discharge_pipe_temp",
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
        "name": "heat_exchanger_mid_temp",
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
        "name": "inv_primary_current",
        "registryID": 0x21,
        "offset": 0,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 10,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "type": "sensor",
        "name": "inv_frequency_rps",
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
        "name": "expansion_valve",
        "registryID": 0x30,
        "offset": 3,
        "signed": False,
        "dataSize": 2,
        "divider": 450.0 / 100.0,
        "endian": Endian.LITTLE,
        "unit_of_measurement": UNIT_PERCENT,
        "state_class": STATE_CLASS_MEASUREMENT,
        "icon": "mdi:pipe-valve"
    },
    {
        "type": "sensor",
        "name": "buh_step_1",
        "registryID": 0x60,
        "offset": 12,
        "handle_lambda": """
            return (data[0] & 0x10) > 0;
        """
    },
    {
        "type": "sensor",
        "name": "buh_step_2",
        "registryID": 0x60,
        "offset": 12,
        "handle_lambda": """
            return (data[0] & 0x08) > 0;
        """
    },
    {
        "type": "sensor",
        "name": "buh_step_bsh",
        "registryID": 0x60,
        "offset": 12,
        "handle_lambda": """
            return (data[0] & 0x20) > 0;
        """
    },
    {
        "type": "sensor",
        "name": "leaving_water_temp_before_buh",
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
        "name": "leaving_water_temp_after_buh",
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
        "name": "t_liquid",
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
        "name": "inlet_water_temp_r4t",
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
        "name": "dhw_tank_temp_r5t",
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

    # 0x62 Not supported by HPSU Compact 2013 + RRLQ008CAV3
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
                    device_class=(sensor_conf.get("device_class") if sensor_conf.get("device_class") != None else sensor._UNDEF),
                    unit_of_measurement=(sensor_conf.get("unit_of_measurement") if sensor_conf.get("unit_of_measurement") != None else sensor._UNDEF),
                    accuracy_decimals=(sensor_conf.get("accuracy_decimals") if sensor_conf.get("accuracy_decimals") != None else sensor._UNDEF),
                    state_class=(sensor_conf.get("state_class") if sensor_conf.get("state_class") != None else sensor._UNDEF),
                    icon=(sensor_conf.get("icon") if sensor_conf.get("icon") != None else sensor._UNDEF)
                ),
            })
        case "binary_sensor":
            entity_schemas.update({
                cv.Optional(name): binary_sensor.binary_sensor_schema(
                    icon=sensor_conf.get("icon", binary_sensor._UNDEF)
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

                match sens_conf.get("type"):
                    case "sensor":
                        entity = await sensor.new_sensor(yaml_sensor_conf)
                    case "binary_sensor":
                        entity = await binary_sensor.new_binary_sensor(yaml_sensor_conf)

                async def handle_lambda():
                    lamb = sens_conf.get("handle_lambda", "return 0;")
                    return await cg.process_lambda(
                        Lambda(lamb),
                        [(u8_ptr, "data")],
                        return_type=cg.uint16,
                    )

                divider = sens_conf.get("divider", 1.0)
                if callable(divider):
                    divider = divider()
                cg.add(var.set_entity(sens_conf.get("name"), [
                    entity,
                    sens_conf.get("registryID"),
                    sens_conf.get("offset"),
                    sens_conf.get("signed", True),
                    sens_conf.get("dataSize", 0),
                    EndianLittle if sens_conf.get("endian") == Endian.LITTLE else EndianBig,
                    divider,
                    await handle_lambda(),
                    "handle_lambda" in sens_conf
                ]))