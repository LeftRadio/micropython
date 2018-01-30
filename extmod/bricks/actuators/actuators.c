
#include "extmod/bricks/brickbase.h"

#if MICROPY_BRICKS_ACTUATORS


STATIC const mp_rom_map_elem_t actuators_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_actuators) },
    { MP_ROM_QSTR(MP_QSTR_DigitalOut), MP_ROM_PTR(&digital_out_type) },
};
STATIC MP_DEFINE_CONST_DICT(actuators_module_globals, actuators_module_globals_table);


const mp_obj_module_t mp_module_actuators = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&actuators_module_globals,
};


#endif // MICROPY_BRICKS_ACTUATORS
