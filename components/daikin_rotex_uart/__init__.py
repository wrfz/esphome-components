import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import *
from esphome.core import Lambda
from esphome.cpp_types import std_ns
from esphome.components import sensor, binary_sensor, text_sensor, uart
from esphome.components.uart import UARTComponent
from enum import Enum
import logging

CODEOWNERS = ["@wrfz"]
AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]
DEPENDENCIES = ["uart"]

translations = {
    "de": {  # German
        "standby": "Standby",
        "heating": "Heizen",
        "cooling": "Kühlen",
        "hot_water": "Warmwasserbereitung",
        "heating_hot_water": "Heizen + Warmwasser",
        "cooling_hot_water": "Kühlen + Warmwasser",
        "mode_of_operating": "Betriebsart",
        "t_liq": "Temperatur Flüssigkeitsleitung",
        "liq_pressure": "Kältemitteldruck",
        "t_liq2": "Kältemitteltemperatur",
        "exv": "Expansionsventil",
        "exch_temp": "Temperatur Lamellenwärmetauscher",
        "fan_speed": "Ventilatordrehzahl",
        "temp_after_compr": "Temperatur nach Kältemittelverdichter",
        "inv_freq": "Kompressor Drehzahl",
        "inv_prim_current": "INV Primary Current",
        "inv_sec_current": "INV Secondary Current",
        "dhw_temp": "DHW tank temp",
        "tv": "Vorlauftemeratur (TV)",
        "tv_bh": "Vorlauftemeratur (TVBH)",
        "tr": "Rücklauftemperatur",
        "outdoor_temp": "Outdoor Air Temp",
        "tgt_cond_temp": "Zielverflüssigungstemperatur",
        "buh1": "Heizstab Stufe 1",
        "buh2": "Heizstab Stufe 2",
        "buh_bsh": "Heizstab Speicher",
        "press_eq": "Druckausgleich",
        "4_way_valve": "4-Wege-Ventil",
        "defrost_operation": "Abtauvorgang"
    },
    "en": {  # English
        "standby": "Standby",
        "heating": "Heating",
        "cooling": "Cooling",
        "hot_water": "Hot water preparation",
        "heating_hot_water": "Heating + Hot water",
        "cooling_hot_water": "Cooling + Hot water",
        "mode_of_operating": "Mode of Operating",
        "t_liq": "Liquid Line Temperature",
        "liq_pressure": "Refrigerant Pressure",
        "t_liq2": "Refrigerant Temperature",
        "exv": "Expansion Valve",
        "exch_temp": "Fin Heat Exchanger Temperature",
        "fan_speed": "Fan Speed",
        "temp_after_compr": "Temperature After Compressor",
        "inv_freq": "Compressor Frequency",
        "inv_prim_current": "INV Primary Current",
        "inv_sec_current": "INV Secondary Current",
        "dhw_temp": "DHW Tank Temperature",
        "tv": "Flow Temperature (TV)",
        "tv_bh": "Flow Temperature (TVBH)",
        "tr": "Return Temperature",
        "outdoor_temp": "Outdoor Air Temperature",
        "tgt_cond_temp": "Target Condensation Temperature",
        "buh1": "Heating Rod Stage 1",
        "buh2": "Heating Rod Stage 2",
        "buh_bsh": "Heating Rod Storage",
        "press_eq": "Pressure Equalization",
        "4_way_valve": "4-Way Valve",
        "defrost_operation": "Defrost Operation"
    },
    "it": {  # Italian
        "standby": "Standby",
        "heating": "Riscaldamento",
        "cooling": "Raffreddamento",
        "hot_water": "Preparazione acqua calda",
        "heating_hot_water": "Riscaldamento + Acqua calda",
        "cooling_hot_water": "Raffreddamento + Acqua calda",
        "mode_of_operating": "Modalità di funzionamento",
        "t_liq": "Temperatura linea liquido",
        "liq_pressure": "Pressione refrigerante",
        "t_liq2": "Temperatura refrigerante",
        "exv": "Valvola di espansione",
        "exch_temp": "Temperatura scambiatore lamellare",
        "fan_speed": "Velocità ventola",
        "temp_after_compr": "Temperatura dopo compressore",
        "inv_freq": "Frequenza compressore",
        "inv_prim_current": "Corrente primaria INV",
        "inv_sec_current": "Corrente secondaria INV",
        "dhw_temp": "Temperatura serbatoio ACS",
        "tv": "Temperatura di mandata (TV)",
        "tv_bh": "Temperatura di mandata (TVBH)",
        "tr": "Temperatura di ritorno",
        "outdoor_temp": "Temperatura aria esterna",
        "tgt_cond_temp": "Temperatura di condensazione target",
        "buh1": "Resistenza di riscaldamento livello 1",
        "buh2": "Resistenza di riscaldamento livello 2",
        "buh_bsh": "Resistenza di riscaldamento accumulo",
        "press_eq": "Equilibrio di pressione",
        "4_way_valve": "Valvola a 4 vie",
        "defrost_operation": "Sbrinamento"
    }
}

_LOGGER = logging.getLogger(__name__)

CONF_LANGUAGE = 'language'
SUPPORTED_LANGUAGES = ['en', 'de', 'it']

# Current language
current_language = "de"
delayed_translate_tag = "DELAYED_TRANSLATE:"

def set_language(lang):
    global current_language
    if lang in translations:
        _LOGGER.info("[Translate] Setting language to '%s'", lang)
        current_language = lang
    else:
        _LOGGER.warning("[Translate] Language '%s' not found in dictionary. Falling back to English.", lang)
        current_language = "en"  # Fallback

def delayed_translate(key: str) -> str:
    return delayed_translate_tag + key

def translate(key: str) -> str:

    global current_language
    lang_translations = translations.get(current_language, translations.get("en", {}))

    if key in lang_translations:
        translated = lang_translations[key]
        _LOGGER.info("[Translate] Key '%s' found in language '%s' -> '%s'",key, current_language, translated)
        return translated

    if "en" in translations and key in translations["en"]:
        _LOGGER.warning(
            "[Translate] Key '%s' not found in language '%s'. Falling back to English.", 
            key, current_language
        )
        return translations["en"][key]
    _LOGGER.error(
        "[Translate] Key '%s' not found in language '%s' or in fallback language 'en'. Returning error message.", 
        key, current_language
    )
    return f"ERROR: Key '{key}' not found"

def apply_delayed_translate(key: str) -> str:
    if isinstance(key, str) and key.startswith(delayed_translate_tag):
        stripped_key = key[len(delayed_translate_tag):]
        return translate(stripped_key)
    return key

def apply_translation_to_mapping(mapping: dict) -> dict:
    return {key: apply_delayed_translate(value) for key, value in mapping.items()}

def apply_translation_to_entityname(yaml_sensor_conf, id):
    if "name" in yaml_sensor_conf and yaml_sensor_conf["name"].strip() == "auto":
        yaml_sensor_conf["name"] = translate(id)


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
        cv.Optional(CONF_LANGUAGE, default="de"): cv.string,        
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

    if CONF_LANGUAGE in config:
        lang = config[CONF_LANGUAGE]
        set_language(lang)

    cg.add(cg.RawStatement('#include "esphome/components/daikin_rotex_uart/unit_converter.h"'))

    u8_ptr = std_ns.class_("uint8_t*")
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if entities := config.get(CONF_ENTITIES):
        for sens_conf in sensor_configuration:
            if yaml_sensor_conf := entities.get(sens_conf.get("name")):
                entity = None

                divider = sens_conf.get("divider", 1.0)
                # translate both map and name (if auto)
                mapping = apply_translation_to_mapping(sens_conf.get("map", {}))
                apply_translation_to_entityname(yaml_sensor_conf,sens_conf.get("name"))                
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