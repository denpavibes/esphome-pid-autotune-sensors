import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, text_sensor
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC

DEPENDENCIES = ["climate"]

pid_ns = cg.esphome_ns.namespace("pid")
PIDClimate = pid_ns.class_("PIDClimate", climate.Climate)

pid_autotune_ns = cg.esphome_ns.namespace("pid_autotune")
PIDAutotuneTextSensor = pid_autotune_ns.class_(
    "PIDAutotuneTextSensor", text_sensor.TextSensor, cg.PollingComponent
)

CONF_CLIMATE_ID = "climate_id"

# We set the default icon and entity_category here
CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(
        icon="mdi:thermometer-auto",
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(PIDAutotuneTextSensor),
            cv.Required(CONF_CLIMATE_ID): cv.use_id(PIDClimate),
        }
    )
    .extend(cv.polling_component_schema("5s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)

    climate_ = await cg.get_variable(config[CONF_CLIMATE_ID])
    cg.add(var.set_climate(climate_))
