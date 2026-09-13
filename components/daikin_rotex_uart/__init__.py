import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import *
from esphome.core import Lambda
from esphome.cpp_types import std_ns
from esphome.components import sensor, binary_sensor, text_sensor, uart
from esphome.components.uart import UARTComponent
from .translations.translate import (
    CONF_LANGUAGE,
    SUPPORTED_LANGUAGES,
    delayed_translate,
    apply_translation_to_mapping,
    set_language,
    check_translations_integrity
)

from enum import Enum
import subprocess
import logging
import os

CODEOWNERS = ["@wrfz"]
AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]
DEPENDENCIES = ["uart"]

_LOGGER = logging.getLogger(__name__)

# Before starting, check the integrity of the translation dictionaries
check_translations_integrity()

daikin_rotex_uart_ns = cg.esphome_ns.namespace("daikin_rotex_uart")
DaikinRotexUARTComponent = daikin_rotex_uart_ns.class_(
    "DaikinRotexUARTComponent", cg.Component, uart.UARTDevice
)

UartSensor = daikin_rotex_uart_ns.class_("UartSensor", sensor.Sensor)
UartTextSensor = daikin_rotex_uart_ns.class_("UartTextSensor", text_sensor.TextSensor)
UartBinarySensor = daikin_rotex_uart_ns.class_("UartBinarySensor", binary_sensor.BinarySensor)

EndianLittle = daikin_rotex_uart_ns.enum('TEntity::Endian::Little')
EndianBig = daikin_rotex_uart_ns.enum('TEntity::Endian::Big')

result = subprocess.run(['git', 'rev-parse', '--short', 'HEAD'], stdout=subprocess.PIPE, text=True, cwd=os.path.dirname(os.path.realpath(__file__)))
git_hash = result.stdout.strip()
_LOGGER.info("Project Git Hash %s", git_hash)

class Endian(Enum):
    LITTLE = 1
    BIG = 2

UNIT_REVOLUTIONS_PER_SECOND = "rps"
UNIT_PRESSURE_BAR = "bar"
UNIT_LITER_PER_HOUR = "L/h"

RRLQ004CAV3 = "RRLQ004CAV3"
RRLQ006CAV3 = "RRLQ006CAV3"
RRLQ008CAV3 = "RRLQ008CAV3"
RRLQ011CAW1 = "RRLQ011CAW1"
ERLQ016CAW1 = "ERLQ016CAW1"
ERGA06EAV3 = "ERGA06EAV3"
ERGA08DAV3 = "ERGA08DAV3"
ERGA08EAV3H = "ERGA08EAV3H"
RRGA08DAV3 = "RRGA08DAV3"
RRLQ008BAV3 = "RRLQ008BAV3"
RDLQ014AA6V3 = "RDLQ014AA6V3"

OUTDOOR_UNIT = {
    RRLQ004CAV3: 1,
    RRLQ006CAV3: 2,
    RRLQ008CAV3: 3,
    RRLQ011CAW1: 4,
    ERLQ016CAW1: 5,
    ERGA06EAV3: 6,
    ERGA08DAV3: 7,
    ERGA08EAV3H: 8,
    RRGA08DAV3: 9,
    RRLQ008BAV3: 10,
    RDLQ014AA6V3: 11
}

current_outdoor_unit = None

fan_divider = {
    RRLQ004CAV3: 0.1,
    RRLQ006CAV3: 0.1,
    RRLQ008CAV3: 0.1,
    RRLQ011CAW1: 0.1 * 0.1,
    ERLQ016CAW1: 0.1 * 0.1,
    ERGA06EAV3: 0.1,
    ERGA08DAV3: 0.1 * 0.1,
    ERGA08EAV3H: 0.1,
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
        "unit_of_measurement": UNIT_PRESSURE_BAR,
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
            return UnitConverter::liquid_pressure_to_temperature(pressureBar);
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
        "name": "voltage",
        "registryID": 0x21,
        "offset": 4,
        "signed": True,
        "dataSize": 2,
        "endian": Endian.LITTLE,
        "divider": 1,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "unit_of_measurement": UNIT_VOLT,
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
        "endian": Endian.LITTLE,
        "unit_of_measurement": "pls",
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
            0x00: delayed_translate("standby"),
            0x01: delayed_translate("heating"),
            0x02: delayed_translate("cooling"),
            0x03: "???",
            0x04: delayed_translate("hot_water"),
            0x05: delayed_translate("heating_hot_water"),
            0x06: delayed_translate("cooling_hot_water"),
        },
        "handle_lambda": """
            return data[0] >> 4;
        """,
        "s": {
            "registryID": 0x55,
            "offset": 0,
            "map": {
                0x00: delayed_translate("fan_only"),
                0x01: delayed_translate("heating"),
                0x02: delayed_translate("cooling"),
                0x03: delayed_translate("auto"),
                0x04: delayed_translate("ventilation"),
                0x05: delayed_translate("auto_cool"),
                0x06: delayed_translate("auto_heat"),
                0x07: delayed_translate("dry"),
                0x08: delayed_translate("aux"),
                0x09: delayed_translate("cooling_storage"),
                0x0A: delayed_translate("heating_storage"),
                0x0B: "UseStrdThrm(cl)1",
                0x0C: "UseStrdThrm(cl)2",
                0x0D: "UseStrdThrm(cl)3",
                0x0E: "UseStrdThrm(cl)4",
                0x0F: "UseStrdThrm(ht)1",
                0x10: "UseStrdThrm(ht)2",
                0x11: "UseStrdThrm(ht)3",
                0x12: "UseStrdThrm(ht)4",
            },
        },
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
        "state_class": STATE_CLASS_MEASUREMENT,
        "update_entities": ["thermal_power", "temperature_spread"],
        "s": {
            "registryID": 0x54,
            "offset": 4,
            "signed": False,
            "divider": 256,
        },
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
        "state_class": STATE_CLASS_MEASUREMENT,
        "s": {
            "registryID": 0x54,
            "offset": 0,
            "signed": False,
            "divider": 256,
        },
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
        "state_class": STATE_CLASS_MEASUREMENT,
        "update_entities": ["thermal_power", "temperature_spread"],
        "s": {
            "registryID": 0x54,
            "offset": 2,
            "signed": False,
            "divider": 256,
        },
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
        "state_class": STATE_CLASS_MEASUREMENT,
        "s": {
            "registryID": 0x54,
            "offset": 8,
            "signed": False,
            "divider": 256,
        },
    },
    {
        "type": "sensor",
        "name": "flow_rate",
        "registryID": 0x62,
        "offset": 9,
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
        "name": "discharge_pressure",
        "registryID": 0x62,
        "offset": 15,
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

########## S-Protocol ##########

# S-protocol wire descriptions live as an "s:" sub-object in the matching entries of
# the shared sensor_configuration list (see t_liq/tr/tv/dhw_temp/mode_of_operating).
# Registry 0x53 relay/flag and 0x55 status signals without an I counterpart are not exposed.

def validate_setoutdoor_unit(value):
    global current_outdoor_unit
    current_outdoor_unit = value
    return value

CONF_ENTITIES = "entities"
CONF_OUTDOR_UNIT = "outdoor_unit"
CONF_PROJECT_GIT_HASH = "project_git_hash"
CONF_PROTOCOL = "protocol"

########## Sensors ##########

CONF_THERMAL_POWER = "thermal_power"
CONF_THERMAL_POWER_RAW = "thermal_power_raw"
CONF_TEMPERATURE_SPREAD = "temperature_spread"
CONF_TEMPERATURE_SPREAD_RAW = "temperature_spread_raw"


entity_schemas = {}
for sensor_conf in sensor_configuration:
    name = sensor_conf.get("name")
    if name in entity_schemas:
        continue

    match sensor_conf.get("type"):
        case "sensor":
            entity_schemas.update({
                cv.Optional(name): sensor.sensor_schema(
                    UartSensor,
                    device_class=(sensor_conf.get("device_class", cv.UNDEFINED)),
                    unit_of_measurement=(sensor_conf.get("unit_of_measurement", cv.UNDEFINED)),
                    accuracy_decimals=(sensor_conf.get("accuracy_decimals", cv.UNDEFINED)),
                    state_class=(sensor_conf.get("state_class", cv.UNDEFINED)),
                    icon=(sensor_conf.get("icon", cv.UNDEFINED))
                ),
            })
        case "binary_sensor":
            entity_schemas.update({
                cv.Optional(name): binary_sensor.binary_sensor_schema(
                    UartBinarySensor,
                    icon=sensor_conf.get("icon", cv.UNDEFINED)
                )
            })
        case "text_sensor":
            entity_schemas.update({
                cv.Optional(name): text_sensor.text_sensor_schema(
                    UartTextSensor,
                    icon=sensor_conf.get("icon", cv.UNDEFINED)
                )
            })

entity_schemas.update({
    ########## Sensors ##########

    cv.Optional(CONF_THERMAL_POWER): sensor.sensor_schema(
        UartSensor,
        device_class=DEVICE_CLASS_POWER,
        unit_of_measurement=UNIT_KILOWATT,
        accuracy_decimals=2,
        state_class=STATE_CLASS_MEASUREMENT
    ),
    cv.Optional(CONF_THERMAL_POWER_RAW): sensor.sensor_schema(
        UartSensor,
        device_class=DEVICE_CLASS_POWER,
        unit_of_measurement=UNIT_KILOWATT,
        accuracy_decimals=2,
        state_class=STATE_CLASS_MEASUREMENT
    ).extend(),
    cv.Optional(CONF_TEMPERATURE_SPREAD): sensor.sensor_schema(
        UartSensor,
        device_class=DEVICE_CLASS_TEMPERATURE,
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        state_class=STATE_CLASS_MEASUREMENT,
        icon="mdi:thermometer-lines"
    ).extend(),
    cv.Optional(CONF_TEMPERATURE_SPREAD_RAW): sensor.sensor_schema(
        UartSensor,
        device_class=DEVICE_CLASS_TEMPERATURE,
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        state_class=STATE_CLASS_MEASUREMENT,
        icon="mdi:thermometer-lines"
    ).extend(),
})

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DaikinRotexUARTComponent),
        cv.Required(CONF_UART_ID): cv.use_id(UARTComponent),
        cv.Required(CONF_OUTDOR_UNIT): cv.ensure_list(cv.enum(OUTDOOR_UNIT), validate_setoutdoor_unit),
        cv.Required(CONF_LANGUAGE): cv.enum(SUPPORTED_LANGUAGES, lower=True, space="_"),
        cv.Optional(CONF_PROTOCOL): cv.one_of("I", "S", upper=True),
        cv.Required(CONF_PROJECT_GIT_HASH): text_sensor.text_sensor_schema(
            icon="mdi:git",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
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

    cg.set_cpp_standard("gnu++23")

    if CONF_LANGUAGE in config:
        lang = config[CONF_LANGUAGE]
        set_language(lang)

    cg.add(cg.RawStatement('#include "esphome/components/daikin_rotex_uart/unit_converter.h"'))

    u8_ptr = std_ns.class_("uint8_t*")
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if text_conf := config.get(CONF_PROJECT_GIT_HASH):
        t = await text_sensor.new_text_sensor(text_conf)
        cg.add(var.set_project_git_hash(t, git_hash))

    if CONF_PROTOCOL in config:
        cg.add(var.set_protocol_from_config(cg.RawExpression(f"esphome::daikin_rotex_uart::TProtocol::{config[CONF_PROTOCOL]}")))


    if entities := config.get(CONF_ENTITIES):
        # One entity object per name. S-protocol metadata, when present, is nested as
        # an "s:" sub-object inside the same sensor_configuration entry, so the entity
        # is created once and configured for both protocols.
        created_entities = {}
        for sens_conf in sensor_configuration:
            name = sens_conf.get("name")
            if not (yaml_sensor_conf := entities.get(name)):
                continue

            def resolve_divider(conf):
                divider = conf.get("divider", 1.0)
                if callable(divider):
                    divider = divider()
                if divider is None:
                    divider = 1.0
                return divider

            entity = created_entities.get(name)
            if entity is None:
                match sens_conf.get("type"):
                    case "sensor":
                        entity = await sensor.new_sensor(yaml_sensor_conf)
                    case "binary_sensor":
                        entity = await binary_sensor.new_binary_sensor(yaml_sensor_conf)
                    case "text_sensor":
                        entity = await text_sensor.new_text_sensor(yaml_sensor_conf)
                created_entities[name] = entity

            async def handle_lambda(conf):
                lamb = conf.get("handle_lambda", "return 0;")
                return await cg.process_lambda(
                    Lambda(lamb),
                    [(u8_ptr, "data")],
                    return_type=cg.uint16,
                )

            def build_args(conf, defaults):
                divider = resolve_divider(conf)
                mapping = apply_translation_to_mapping(conf.get("map", {}))
                str_map = "|".join([f"0x{int(key * divider) & 0xFFFF :02X}:{value}" for key, value in mapping.items()])
                return [
                    entity,
                    name,
                    conf.get("registryID"),
                    conf.get("offset"),
                    conf.get("signed", defaults["signed"]),
                    conf.get("dataSize", defaults["dataSize"]),
                    EndianLittle if conf.get("endian") == Endian.LITTLE else EndianBig,
                    divider,
                    conf.get("accuracy_decimals", 0),
                    conf.get("update_entities", []),
                ], str_map

            i_args, i_str_map = build_args(sens_conf, {"signed": True, "dataSize": 0})
            i_args.extend([await handle_lambda(sens_conf), "handle_lambda" in sens_conf])
            cg.add(entity.set_entity(i_args))
            if i_str_map:
                cg.add(entity.set_map(i_str_map))

            if s_conf := sens_conf.get("s"):
                # Fields not named in "s:" inherit from the I entry, so identical
                # values (endian, dataSize, accuracy, update_entities, ...) are not
                # repeated. map/handle_lambda stay S-only.
                merged = {k: v for k, v in sens_conf.items() if k not in ("map", "handle_lambda", "s")}
                merged.update(s_conf)
                s_args, s_str_map = build_args(merged, {"signed": False, "dataSize": 1})
                s_args.extend([await handle_lambda(s_conf), "handle_lambda" in s_conf])
                cg.add(entity.set_entity_s(s_args))
                if s_str_map:
                    cg.add(entity.set_map_s(s_str_map))

        for entity in created_entities.values():
            cg.add(var.add_entity(entity))

        ########## Sensors ##########

        if yaml_sensor_conf := entities.get(CONF_THERMAL_POWER):
            sens = await sensor.new_sensor(yaml_sensor_conf)
            cg.add(sens.set_id(CONF_THERMAL_POWER))
            cg.add(var.set_thermal_power_sensor(sens))
        if yaml_sensor_conf := entities.get(CONF_THERMAL_POWER_RAW):
            sens = await sensor.new_sensor(yaml_sensor_conf)
            cg.add(sens.set_id(CONF_THERMAL_POWER_RAW))
            cg.add(var.set_thermal_power_sensor_raw(sens))
        if yaml_sensor_conf := entities.get(CONF_TEMPERATURE_SPREAD):
            sens = await sensor.new_sensor(yaml_sensor_conf)
            cg.add(sens.set_id(CONF_TEMPERATURE_SPREAD))
            cg.add(var.set_temperature_spread(sens))
        if yaml_sensor_conf := entities.get(CONF_TEMPERATURE_SPREAD_RAW):
            sens = await sensor.new_sensor(yaml_sensor_conf)
            cg.add(sens.set_id(CONF_TEMPERATURE_SPREAD_RAW))
            cg.add(var.set_temperature_spread_raw(sens))

