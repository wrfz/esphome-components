import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import *
from esphome.components import sensor, uart
from esphome.components.uart import UARTComponent
from enum import Enum

CODEOWNERS = ["@MrSuicideParrot"]
DEPENDENCIES = ["uart"]

daikin_rotex_uart_ns = cg.esphome_ns.namespace("daikin_rotex_uart")
DaikinRotexUARTComponent = daikin_rotex_uart_ns.class_(
    "DaikinRotexUARTComponent", sensor.Sensor, cg.Component, uart.UARTDevice
)
EndianLittle = daikin_rotex_uart_ns.enum('TMessage::Endian::Little')
EndianBig = daikin_rotex_uart_ns.enum('TMessage::Endian::Big')

class Endian(Enum):
    LITTLE = 1
    BIG = 2

UNIT_REVOLUTIONS_PER_SECOND = "rps"

sensor_configuration = [
    {
        "name": "operation_mode",       # doesn't work for HPSU compact 508 / 8kW
        "registryID": 0x10,
        "offset": 0,
        "dataSize": 1,
    },
    {
        "name": "defrost_opreration",   # doesn't work for HPSU compact 508 / 8kW
        "registryID": 0x10,
        "offset": 1,
        "dataSize": 1,
    },
    {
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
        "name": "voltage_n_phase",
        "registryID": 0x21,
        "offset": 4,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "name": "inv_frequency_rps",
        "registryID": 0x30,
        "offset": 0,
        "signed": False,
        "dataSize": 1,
        "unit_of_measurement": UNIT_REVOLUTIONS_PER_SECOND,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "name": "fan_speed",
        "registryID": 0x30,
        "offset": 1,
        "signed": False,
        "dataSize": 1,
        "divider": 0.1,
        "unit_of_measurement": UNIT_REVOLUTIONS_PER_MINUTE,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "name": "expansion_valve",
        "registryID": 0x30,
        "offset": 3,
        "signed": False,
        "dataSize": 2,
        "divider": 450.0 / 100.0,
        "endian": Endian.LITTLE,
        "unit_of_measurement": UNIT_PERCENT,
        "state_class": STATE_CLASS_MEASUREMENT
    },
    {
        "name": "leaving_water_temp_before_buh_r1t",
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
        "name": "leaving_water_temp_after_buh_r2t",
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
        "name": "refrig_temp_liquid_side_r3t",
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
    },
    {
        "name": "refrigerant_pressure",     # doesn't work for HPSU compact 508 / 8kW
        "registryID": 0x62,
        "offset": 15,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE
    },
    {
        "name": "refrigerant_temperature",     # doesn't work for HPSU compact 508 / 8kW
        "registryID": 0x62,
        "offset": 15,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE
    },
]

CONF_ENTITIES = "entities"

entity_schemas = {}
for sensor_conf in sensor_configuration:
    name = sensor_conf.get("name")
    entity_schemas.update({
        cv.Optional(name): sensor.sensor_schema(
            device_class=(sensor_conf.get("device_class") if sensor_conf.get("device_class") != None else sensor._UNDEF),
            unit_of_measurement=(sensor_conf.get("unit_of_measurement") if sensor_conf.get("unit_of_measurement") != None else sensor._UNDEF),
            accuracy_decimals=(sensor_conf.get("accuracy_decimals") if sensor_conf.get("accuracy_decimals") != None else sensor._UNDEF),
            state_class=(sensor_conf.get("state_class") if sensor_conf.get("state_class") != None else sensor._UNDEF),
            icon=(sensor_conf.get("icon") if sensor_conf.get("icon") != None else sensor._UNDEF)
        ),
    })

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DaikinRotexUARTComponent),
        cv.Required(CONF_UART_ID): cv.use_id(UARTComponent),
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
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if entities := config.get(CONF_ENTITIES):
        for sens_conf in sensor_configuration:
            if yaml_sensor_conf := entities.get(sens_conf.get("name")):
                entity = await sensor.new_sensor(yaml_sensor_conf)
                cg.add(var.set_entity(sens_conf.get("name"), [
                    entity,
                    sens_conf.get("registryID"),
                    sens_conf.get("offset"),
                    sens_conf.get("signed"),
                    sens_conf.get("dataSize"),
                    EndianLittle if sens_conf.get("endian") == Endian.LITTLE else EndianBig,
                    sens_conf.get("divider", 1.0),
                ]))