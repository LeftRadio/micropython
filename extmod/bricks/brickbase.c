

#include "extmod/bricks/brickbase.h"


#if MICROPY_BRICKS_BASE


STATIC mp_obj_t bricks_groups(void) {
    mp_obj_t group_list[] = {
        MP_ROM_QSTR(MP_QSTR_actuators),
        MP_ROM_QSTR(MP_QSTR_sensors),
        MP_ROM_QSTR(MP_QSTR_scenaries),
    };
    return mp_obj_new_list(3, group_list);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(bricks_groups_obj, bricks_groups);


mp_obj_t brickbase_common_update_callback = MP_OBJ_NULL;

STATIC mp_obj_t set_callback(mp_obj_t callback) {
    brickbase_common_update_callback = callback;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(set_callback_obj, set_callback);


STATIC const mp_rom_map_elem_t brick_base_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_bricks) },
    { MP_ROM_QSTR(MP_QSTR_groups), MP_ROM_PTR(&bricks_groups_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_callback), MP_ROM_PTR(&set_callback_obj) },
};
STATIC MP_DEFINE_CONST_DICT(brick_base_module_globals, brick_base_module_globals_table);


const mp_obj_module_t mp_module_bricks = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&brick_base_module_globals,
};

#endif // MICROPY_BRICKS_BASE
