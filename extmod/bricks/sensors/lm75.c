

#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "extmod/machine_i2c.h"
#include "extmod/bricks/brickbase.h"


extern mp_obj_t bricks_sensors_req_i2c_peripheral(mp_int_t id, mp_int_t scl, mp_int_t sda, mp_int_t freq);


typedef struct _lm75_obj_t {
    mp_obj_base_t base;
    const char *name;
    size_t name_len;
    bool enable;
    mp_obj_t i2c_obj;
    mp_int_t i2c_id;
    mp_int_t i2c_scl;
    mp_int_t i2c_sda;
    mp_int_t i2c_freq;
    mp_int_t i2c_addr;
    mp_int_t thermo_reg_data;
    mp_int_t data;
} lm75_obj_t;

const mp_obj_type_t lm75_type;


STATIC int _lm75_read_temperature(lm75_obj_t* self) {
    //
    mp_obj_base_t *i2c_obj = (mp_obj_base_t*)MP_OBJ_TO_PTR(self->i2c_obj);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)(i2c_obj->type->protocol);
    //
    vstr_t vstr;
    vstr_init_len(&vstr, 2);
    bool stop = false;

    int ret = i2c_p->readfrom(i2c_obj, self->i2c_addr, (uint8_t*)vstr.buf, vstr.len, stop);

    if (ret < 0) {
        self->data = -65535;
    }
    else {
        self->data = vstr.buf[0];
    }

    return ret;
}


STATIC void lm75_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    lm75_obj_t *self = self_in;
    mp_printf(print, "Bricks Sensor LM75(%p)", &self->base);
}


STATIC void lm75_init_helper(lm75_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_name, ARG_enable, ARG_i2c_id, ARG_i2c_sda, ARG_i2c_scl, ARG_i2c_freq, ARG_i2c_addr, ARG_thermo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_enable, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_i2c_id, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_i2c_sda, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_i2c_scl, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_i2c_freq, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 10000} },
        { MP_QSTR_i2c_addr, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_thermo, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -65535} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[ARG_name].u_obj != MP_OBJ_NULL) {
        self->name = mp_obj_str_get_data(args[ARG_name].u_obj, &self->name_len);
    }
    if (args[ARG_enable].u_obj != MP_OBJ_NULL) {
        self->enable = mp_obj_is_true(args[ARG_enable].u_obj);
    }
    if (args[ARG_i2c_sda].u_int != -1 && args[ARG_i2c_scl].u_int != -1 && args[ARG_i2c_addr].u_int != -1) {
        self->i2c_id = args[ARG_i2c_id].u_int;
        self->i2c_scl = args[ARG_i2c_scl].u_int;
        self->i2c_sda = args[ARG_i2c_sda].u_int;
        self->i2c_freq = args[ARG_i2c_freq].u_int;
        self->i2c_addr = args[ARG_i2c_addr].u_int;
        self->i2c_obj = bricks_sensors_req_i2c_peripheral(
            self->i2c_id,
            self->i2c_scl,
            self->i2c_sda,
            self->i2c_freq
        );
    }
    if (args[ARG_thermo].u_int != -65535) {
        self->thermo_reg_data = args[ARG_thermo].u_int;
    }
}


STATIC mp_obj_t lm75_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 255, true);
    // create
    lm75_obj_t *self = m_new_obj(lm75_obj_t);
    //
    self->base.type = &lm75_type;
    self->name = "lm75-1";
    self->name_len = 6;
    self->enable = false;
    self->i2c_id = -1;
    self->i2c_scl = -1;
    self->i2c_sda = -1;
    self->i2c_freq = 10000;
    self->i2c_addr = -1;
    self->thermo_reg_data = -65535;
    self->data = -65535;
    //
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    lm75_init_helper(self, n_args, args, &kw_args);
    //
    return MP_OBJ_FROM_PTR(self);
}


STATIC mp_obj_t lm75_init(mp_obj_t self_in) {
    // first read for dummy data
    _lm75_read_temperature( (lm75_obj_t*)self_in );
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lm75_init_obj, lm75_init);


STATIC mp_obj_t lm75_update(mp_obj_t self_in, mp_obj_t seconds_in, mp_obj_t interval_in) {
    lm75_obj_t *self = (lm75_obj_t*)self_in;
    int ret = _lm75_read_temperature(self);
    if ( (ret >= 0) && (brickbase_common_update_callback != MP_OBJ_NULL) ) {
        mp_call_function_2(
            brickbase_common_update_callback,
            self,
            mp_obj_new_int(self->data)
        );
    }
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(lm75_update_obj, lm75_update);


STATIC mp_obj_t lm75_name(mp_obj_t self_in) {
    lm75_obj_t *self = (lm75_obj_t*)self_in;
    return mp_obj_new_str(self->name, self->name_len);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lm75_name_obj, lm75_name);


STATIC mp_obj_t lm75_group(mp_obj_t self_in) {
    return MP_ROM_QSTR(MP_QSTR_sensors);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lm75_group_obj, lm75_group);


STATIC mp_obj_t lm75_properties(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {

    lm75_obj_t *self = (lm75_obj_t*)args[0];
    mp_uint_t kw_len = kw_args->alloc;

    if (kw_len == 0) {
        mp_obj_dict_t *dict = mp_obj_new_dict(9);
        mp_map_elem_t *t = dict->map.table;
        t[0].key = MP_ROM_QSTR(MP_QSTR_classname);  t[0].value = MP_ROM_QSTR(MP_QSTR_LM75);
        t[1].key = MP_ROM_QSTR(MP_QSTR_name);       t[1].value = mp_obj_new_str(self->name, self->name_len);
        t[2].key = MP_ROM_QSTR(MP_QSTR_enable);     t[2].value = mp_obj_new_bool(self->enable);
        t[3].key = mp_obj_new_str("i2c_id", 6);     t[3].value = mp_obj_new_int(self->i2c_id);
        t[4].key = mp_obj_new_str("i2c_scl", 7);    t[4].value = mp_obj_new_int(self->i2c_scl);
        t[5].key = mp_obj_new_str("i2c_sda", 7);    t[5].value = mp_obj_new_int(self->i2c_sda);
        t[6].key = mp_obj_new_str("i2c_freq", 8);   t[6].value = mp_obj_new_int(self->i2c_freq);
        t[7].key = mp_obj_new_str("i2c_addr", 8);   t[7].value = mp_obj_new_int(self->i2c_addr);
        t[8].key = mp_obj_new_str("thermo", 6);     t[8].value = mp_obj_new_int(self->thermo_reg_data);
        return MP_OBJ_FROM_PTR(dict);
    }

    lm75_init_helper(self, n_args-1, args+1, kw_args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(lm75_properties_obj, 1, lm75_properties);


STATIC const mp_rom_map_elem_t lm75_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&lm75_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_update), MP_ROM_PTR(&lm75_update_obj) },
    { MP_ROM_QSTR(MP_QSTR_name), MP_ROM_PTR(&lm75_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_group), MP_ROM_PTR(&lm75_group_obj) },
    { MP_ROM_QSTR(MP_QSTR_properties), MP_ROM_PTR(&lm75_properties_obj) },
};
STATIC MP_DEFINE_CONST_DICT(lm75_locals_dict, lm75_locals_dict_table);


const mp_obj_type_t lm75_type = {
    { &mp_type_type },
    .name = MP_QSTR_LM75,
    .print = lm75_print,
    .make_new = lm75_make_new,
    .locals_dict = (mp_obj_dict_t*)&lm75_locals_dict,
};
