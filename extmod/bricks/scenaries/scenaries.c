
#include "extmod/bricks/brickbase.h"

#if MICROPY_BRICKS_SCENARIES


STATIC const mp_rom_map_elem_t scenaries_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_scenaries) },
    { MP_ROM_QSTR(MP_QSTR_UserTimer), MP_ROM_PTR(&timer_type) },
    { MP_ROM_QSTR(MP_QSTR_Regulator), MP_ROM_PTR(&regulator_type) },
};
STATIC MP_DEFINE_CONST_DICT(scenaries_module_globals, scenaries_module_globals_table);


const mp_obj_module_t mp_module_scenaries = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&scenaries_module_globals,
};


#endif // MICROPY_BRICKS_SCENARIES
