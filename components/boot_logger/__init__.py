import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome.const import (
    CONF_ID
)

DEPENDENCIES = ["logger"]

CODEOWNERS = ["@wrfz"]
boot_logger_ns = cg.esphome_ns.namespace("boot_logger")

CONF_HEAP_LIMIT = "heap_limit"
CONF_DUMP_LINES_PER_LOOP = "dump_lines_per_loop"
CONF_CONNECT_DELAY = "connect_delay"

BootLogger = boot_logger_ns.class_("BootLogger", cg.Component)

CONFIG_SCHEMA = cv.All(
   cv.Schema({
      cv.GenerateID(CONF_ID): cv.declare_id(BootLogger),
      cv.Required(CONF_HEAP_LIMIT): cv.int_range(min=1024, max=1048576),
      cv.Optional(CONF_DUMP_LINES_PER_LOOP, default=20): cv.int_range(min=1, max=100),
      cv.Optional(CONF_CONNECT_DELAY, default="0ms"): cv.All(
         cv.positive_time_period_milliseconds,
         cv.Range(min=core.TimePeriod(milliseconds=0), max=core.TimePeriod(milliseconds=10000)),
      ),
   })
)

async def to_code(config):
   var = cg.new_Pvariable(config[CONF_ID], config[CONF_HEAP_LIMIT], config[CONF_DUMP_LINES_PER_LOOP], config[CONF_CONNECT_DELAY])
   await cg.register_component(var, config)
