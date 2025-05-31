from typing import Any

import esphome.config_validation as cv
from esphome.components import sensor
from .. import const, schema, validate, generate

DEPENDENCIES = [const.OPENTHERM]
COMPONENT_TYPE = const.SENSOR

MSG_DATA_TYPES = {
    "u8_lb",
    "u8_hb",
    "s8_lb",
    "s8_hb",
    "u8_lb_60",
    "u8_hb_60",
    "u16",
    "s16",
    "f88",
}


def get_entity_validation_schema(entity: schema.SensorSchema) -> cv.Schema:
    kwargs = {}

    if entity.unit_of_measurement is not None:
        kwargs["unit_of_measurement"] = entity.unit_of_measurement

    if entity.accuracy_decimals is not None:
        kwargs["accuracy_decimals"] = entity.accuracy_decimals

    if entity.device_class is not None:
        kwargs["device_class"] = entity.device_class

    if entity.icon is not None:
        kwargs["icon"] = entity.icon

    if entity.state_class is not None:
        kwargs["state_class"] = entity.state_class

    return sensor.sensor_schema(**kwargs).extend(
        {
            cv.Optional(const.CONF_DATA_TYPE): cv.one_of(*MSG_DATA_TYPES),
        }
    )


CONFIG_SCHEMA = validate.create_component_schema(
    schema.SENSORS, get_entity_validation_schema
)


async def to_code(config: dict[str, Any]) -> None:
    await generate.component_to_code(
        COMPONENT_TYPE,
        schema.SENSORS,
        sensor.Sensor,
        generate.create_only_conf(sensor.new_sensor),
        config,
    )
