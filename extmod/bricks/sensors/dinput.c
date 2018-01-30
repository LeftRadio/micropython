
#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "extmod/virtpin.h"
#include "extmod/bricks/brickbase.h"


typedef struct _digital_input_obj_t {
    mp_obj_base_t base;
    const char *name;
    size_t name_len;
    bool enable;
    mp_uint_t index;
    mp_obj_t pin;
} digital_input_obj_t;

const mp_obj_type_t digital_input_type;


STATIC void digital_input_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    digital_input_obj_t *self = self_in;
    mp_printf(print, "Bricks sensor digital input(%p)", &self->base);
}


STATIC void digital_input_init_helper(digital_input_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_name, ARG_enable, ARG_index };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_enable, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_index, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[ARG_name].u_obj != MP_OBJ_NULL) {
        self->name = mp_obj_str_get_data(args[ARG_name].u_obj, &self->name_len);
    }
    if (args[ARG_enable].u_obj != MP_OBJ_NULL) {
        self->enable = mp_obj_is_true(args[ARG_enable].u_obj);
    }
    if (args[ARG_index].u_int != -1) {
        self->index = args[ARG_index].u_int;
        mp_obj_t pargs[1] = { mp_obj_new_int(self->index) };
        self->pin = mp_pin_make_new(NULL, 1, 0, pargs);
        mp_virtual_pin_input(self->pin);
    }
}


STATIC mp_obj_t digital_input_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 255, true);
    // create
    digital_input_obj_t *self = m_new_obj(digital_input_obj_t);
    //
    self->base.type = &digital_input_type;
    self->name = "din-1";
    self->name_len = 5;
    self->enable = false;
    self->index = -1;
    self->pin = MP_OBJ_NULL;
    //
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    digital_input_init_helper(self, n_args, args, &kw_args);
    //
    return MP_OBJ_FROM_PTR(self);
}


STATIC mp_obj_t digital_input_init(mp_obj_t self_in) {
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(digital_input_init_obj, digital_input_init);


STATIC mp_obj_t digital_input_update(mp_obj_t self_in, mp_obj_t seconds_in, mp_obj_t interval_in) {

    digital_input_obj_t *self = (digital_input_obj_t*)self_in;

    int res = -1;

    if (self->enable && self->pin != MP_OBJ_NULL) {
        res = mp_virtual_pin_read(self->pin);

        if ( brickbase_common_update_callback != MP_OBJ_NULL ) {
            mp_call_function_2(
                brickbase_common_update_callback,
                self,
                mp_obj_new_int(res)
            );
        }
    }

    return mp_obj_new_int(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(digital_input_update_obj, digital_input_update);


STATIC mp_obj_t digital_input_name(mp_obj_t self_in) {
    digital_input_obj_t *self = (digital_input_obj_t*)self_in;
    return mp_obj_new_str(self->name, self->name_len);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(digital_input_name_obj, digital_input_name);


STATIC mp_obj_t digital_input_group(mp_obj_t self_in) {
    return MP_ROM_QSTR(MP_QSTR_sensors);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(digital_input_group_obj, digital_input_group);


STATIC mp_obj_t digital_input_properties(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {

    digital_input_obj_t *self = (digital_input_obj_t*)args[0];
    mp_uint_t kw_len = kw_args->alloc;

    if (kw_len == 0) {
        mp_obj_dict_t *dict = mp_obj_new_dict(4);
        mp_map_elem_t *t = dict->map.table;
        t[0].key = MP_ROM_QSTR(MP_QSTR_classname);      t[0].value = MP_ROM_QSTR(MP_QSTR_LM75);
        t[1].key = MP_ROM_QSTR(MP_QSTR_name);           t[1].value = mp_obj_new_str(self->name, self->name_len);
        t[2].key = MP_ROM_QSTR(MP_QSTR_enable);         t[2].value = mp_obj_new_bool(self->enable);
        t[3].key = mp_obj_new_str("index", 5);          t[3].value = mp_obj_new_int(self->index);
        return MP_OBJ_FROM_PTR(dict);
    }

    digital_input_init_helper(self, n_args-1, args+1, kw_args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(digital_input_properties_obj, 1, digital_input_properties);


STATIC const mp_rom_map_elem_t digital_input_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&digital_input_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_update), MP_ROM_PTR(&digital_input_update_obj) },
    { MP_ROM_QSTR(MP_QSTR_name), MP_ROM_PTR(&digital_input_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_group), MP_ROM_PTR(&digital_input_group_obj) },
    { MP_ROM_QSTR(MP_QSTR_properties), MP_ROM_PTR(&digital_input_properties_obj) },
};
STATIC MP_DEFINE_CONST_DICT(digital_input_locals_dict, digital_input_locals_dict_table);


const mp_obj_type_t digital_input_type = {
    { &mp_type_type },
    .name = MP_QSTR_DigitalInput,
    .print = digital_input_print,
    .make_new = digital_input_make_new,
    .locals_dict = (mp_obj_dict_t*)&digital_input_locals_dict,
};
