import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch, climate
from esphome.const import CONF_ID, ENTITY_CATEGORY_CONFIG

DEPENDENCIES = ["climate"]

pid_ns = cg.esphome_ns.namespace("pid")
PIDClimate = pid_ns.class_("PIDClimate", climate.Climate)

pid_autotune_ns = cg.esphome_ns.namespace("pid_autotune")
PIDAutotuneSwitch = pid_autotune_ns.class_(
    "PIDAutotuneSwitch", switch.Switch, cg.PollingComponent
)

CONF_CLIMATE_ID = "climate_id"

CONFIG_SCHEMA = (
    switch.switch_schema(
        PIDAutotuneSwitch,
        icon="mdi:tune",
        entity_category=ENTITY_CATEGORY_CONFIG,
    )
    .extend(
        {
            cv.Required(CONF_CLIMATE_ID): cv.use_id(PIDClimate),
        }
    )
    .extend(cv.polling_component_schema("5s"))
)


async def to_code(config):
    var = await switch.new_switch(config)
    await cg.register_component(var, config)

    climate_ = await cg.get_variable(config[CONF_CLIMATE_ID])
    cg.add(var.set_climate(climate_))
