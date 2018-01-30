

#include "py/obj.h"
#include "py/mperrno.h"
#include "py/runtime.h"
#include "extmod/bricks/brickbase.h"


typedef struct _regulator_obj_t {
    mp_obj_base_t base;
    const char *name;
    size_t name_len;
    bool enable;
    bool binded;
    mp_int_t intent;
    mp_uint_t hysteresis;
    bool inverted;
    bool out_state;
    mp_int_t source_data;
} regulator_obj_t;

const mp_obj_type_t regulator_type;


STATIC void regulator_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    regulator_obj_t *self = self_in;
    mp_printf(print, "Bricks Scenaries regulator(%p)", &self->base);
}


STATIC void regulator_init_helper(regulator_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_name, ARG_enable, ARG_binded, ARG_intent, ARG_hysteresis, ARG_inverted };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_enable, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_binded, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_intent, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -65535} },
        { MP_QSTR_hysteresis, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_inverted, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[ARG_name].u_obj != MP_OBJ_NULL) {
        self->name = mp_obj_str_get_data(args[ARG_name].u_obj, &self->name_len);
    }
    if (args[ARG_enable].u_obj != MP_OBJ_NULL) {
        self->enable = mp_obj_is_true(args[ARG_enable].u_obj);
    }
    if (args[ARG_binded].u_obj != MP_OBJ_NULL) {
        self->binded = mp_obj_is_true(args[ARG_binded].u_obj);
    }
    if (args[ARG_intent].u_int != -65535) {
        self->intent = args[ARG_intent].u_int;
    }
    if (args[ARG_hysteresis].u_int != -1) {
        self->hysteresis = args[ARG_hysteresis].u_int;
    }
    if (args[ARG_inverted].u_obj != MP_OBJ_NULL) {
        self->inverted = mp_obj_is_true(args[ARG_inverted].u_obj);
    }
}


STATIC mp_obj_t regulator_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 255, true);
    // create
    regulator_obj_t *self = m_new_obj(regulator_obj_t);
    //
    self->base.type = &regulator_type;
    self->name = "regulator-1";
    self->name_len = 11;
    self->enable = false;
    self->binded = false;
    self->intent = -65535;
    self->hysteresis = -1;
    self->inverted = false;
    self->source_data = -65535;
    self->out_state = false;
    //
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    regulator_init_helper(self, n_args, args, &kw_args);
    //
    return MP_OBJ_FROM_PTR(self);
}


STATIC mp_obj_t regulator_init(mp_obj_t self_in) {
    return mp_const_none;
}


STATIC mp_obj_t regulator_update(mp_obj_t self_in, mp_obj_t seconds, mp_obj_t interval) {

    regulator_obj_t *self = (regulator_obj_t*)self_in;

    if (self->enable && self->source_data != -65535) {

        mp_int_t low_intent = self->intent - self->hysteresis;
        mp_int_t hight_intent = self->intent + self->hysteresis;

        bool change_state = false;

        if (self->out_state && self->source_data >= hight_intent) {
            change_state = true;
        }
        else if ((!self->out_state) && self->source_data <= low_intent) {
            change_state = true;
        }

        if (!change_state) {
            return mp_const_none;
        }
        self->out_state ^= change_state;
    }
    else {
        if (self->out_state){
            self->out_state = false;
        }
        else {
            return mp_const_none;
        }
    }

    if (brickbase_common_update_callback != MP_OBJ_NULL) {
        mp_call_function_2(
            brickbase_common_update_callback,
            self,
            mp_obj_new_bool(self->out_state ^ self->inverted)
        );
    }

    return mp_const_none;
}


STATIC mp_obj_t regulator_update_slot(mp_obj_t self_in, mp_obj_t sender, mp_obj_t data) {
    regulator_obj_t *self = (regulator_obj_t*)self_in;
    self->source_data = mp_obj_get_int(data);
    return mp_const_none;
}


STATIC mp_obj_t regulator_name(mp_obj_t self_in) {
    regulator_obj_t *self = (regulator_obj_t*)self_in;
    return mp_obj_new_str(self->name, self->name_len);
}


STATIC mp_obj_t regulator_group(mp_obj_t self_in) {
    return MP_ROM_QSTR(MP_QSTR_scenaries);
}


STATIC mp_obj_t regulator_properties(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {

    regulator_obj_t *self = (regulator_obj_t*)args[0];
    mp_uint_t kw_len = kw_args->alloc;

    if (kw_len == 0) {
        mp_obj_dict_t *dict = mp_obj_new_dict(7);
        mp_map_elem_t *t = dict->map.table;
        t[0].key = MP_ROM_QSTR(MP_QSTR_classname);      t[0].value = MP_ROM_QSTR(MP_QSTR_LM75);
        t[1].key = MP_ROM_QSTR(MP_QSTR_name);           t[1].value = mp_obj_new_str(self->name, self->name_len);
        t[2].key = MP_ROM_QSTR(MP_QSTR_enable);         t[2].value = mp_obj_new_bool(self->enable);
        t[3].key = MP_ROM_QSTR(MP_QSTR_binded);         t[3].value = mp_obj_new_bool(self->binded);
        t[4].key = mp_obj_new_str("intent", 6);         t[4].value = mp_obj_new_int(self->intent);
        t[5].key = mp_obj_new_str("hysteresis", 10);    t[5].value = mp_obj_new_int(self->hysteresis);
        t[6].key = mp_obj_new_str("inverted", 8);       t[6].value = mp_obj_new_bool(self->inverted);
        return MP_OBJ_FROM_PTR(dict);
    }

    regulator_init_helper(self, n_args-1, args+1, kw_args);
    return mp_const_none;
}


STATIC MP_DEFINE_CONST_FUN_OBJ_1(regulator_init_obj, regulator_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(regulator_update_obj, regulator_update);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(regulator_update_slot_obj, regulator_update_slot);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(regulator_name_obj, regulator_name);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(regulator_group_obj, regulator_group);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(regulator_properties_obj, 1, regulator_properties);

STATIC const mp_rom_map_elem_t regulator_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&regulator_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_update), MP_ROM_PTR(&regulator_update_obj) },
    { MP_ROM_QSTR(MP_QSTR_update_slot), MP_ROM_PTR(&regulator_update_slot_obj) },
    { MP_ROM_QSTR(MP_QSTR_name), MP_ROM_PTR(&regulator_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_group), MP_ROM_PTR(&regulator_group_obj) },
    { MP_ROM_QSTR(MP_QSTR_properties), MP_ROM_PTR(&regulator_properties_obj) },
};
STATIC MP_DEFINE_CONST_DICT(regulator_locals_dict, regulator_locals_dict_table);


const mp_obj_type_t regulator_type = {
    { &mp_type_type },
    .name = MP_QSTR_Regulator,
    .print = regulator_print,
    .make_new = regulator_make_new,
    .locals_dict = (mp_obj_dict_t*)&regulator_locals_dict,
};
