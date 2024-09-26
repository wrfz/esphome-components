import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import *
from esphome.components import sensor, uart
from esphome.components.uart import UARTComponent

CODEOWNERS = ["@MrSuicideParrot"]
DEPENDENCIES = ["uart"]

daikin_rotex_uart_ns = cg.esphome_ns.namespace("daikin_rotex_uart")
DaikinRotexUARTComponent = daikin_rotex_uart_ns.class_(
    "DaikinRotexUARTComponent", sensor.Sensor, cg.Component, uart.UARTDevice
)

sensor_configuration = [
    {
        "name": "inv_primary_current",
        "convid": 0x21,
        "offset": 0,
        "registryID": 105,
        "dataSize": 2,
        "dataType": -1
    }
]

CONF_ENTITIES = "entities"

entity_schemas = {}
for sensor_conf in sensor_configuration:
    name = sensor_conf.get("name")
    entity_schemas.update({
        cv.Optional(name): sensor.sensor_schema(
            #device_class=(sensor_conf.get("device_class") if sensor_conf.get("device_class") != None else sensor._UNDEF),
            #unit_of_measurement=sensor_conf.get("unit_of_measurement"),
            #accuracy_decimals=sensor_conf.get("accuracy_decimals"),
            #state_class=sensor_conf.get("state_class"),
            #icon=sensor_conf.get("icon", sensor._UNDEF)
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
                    sens_conf.get("convid"),
                    sens_conf.get("offset"),
                    sens_conf.get("registryID"),
                    sens_conf.get("dataSize"),
                    sens_conf.get("dataType"),
                ]))