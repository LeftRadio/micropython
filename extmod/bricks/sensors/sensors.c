
#include <stdio.h>
#include <stdint.h>

#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "extmod/virtpin.h"
#include "extmod/machine_i2c.h"
#include "extmod/bricks/brickbase.h"

#if MICROPY_BRICKS_SENSORS


extern mp_obj_t machine_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);


mp_map_elem_t _hal_i2c[5] = {
        {MP_OBJ_NULL, MP_OBJ_NULL},
        {MP_OBJ_NULL, MP_OBJ_NULL},
        {MP_OBJ_NULL, MP_OBJ_NULL},
        {MP_OBJ_NULL, MP_OBJ_NULL},
        {MP_OBJ_NULL, MP_OBJ_NULL}
    };


STATIC mp_obj_t _search_i2c_periph(mp_int_t id, mp_int_t* index) {
    *index = 0;
    for (int i = 0; i < 5; i++) {
        if (_hal_i2c[i].key != MP_OBJ_NULL && _hal_i2c[i].value != MP_OBJ_NULL) {
            *index = i + 1;
            if ( id == mp_obj_get_int(_hal_i2c[i].key) ) {
                return _hal_i2c[i].value;
            }
        }
    }
    return MP_OBJ_NULL;
}


mp_obj_t bricks_sensors_req_i2c_peripheral(mp_int_t id, mp_int_t scl, mp_int_t sda, mp_int_t freq) {
    // search exist
    mp_int_t next_index = 0;
    mp_obj_t i2c_inst = _search_i2c_periph(id, &next_index);

    if (i2c_inst != MP_OBJ_NULL) {
        return i2c_inst;
    }
    else if (next_index == 5) {
        mp_raise_ValueError("error create I2C peripheral, maximum of instances used");
    }

    // create SDA/SCL pins
    mp_obj_t pargs[1] = { mp_obj_new_int(scl) };
    mp_obj_t scl_pin = mp_pin_make_new(NULL, 1, 0, pargs);
    pargs[0] = mp_obj_new_int(sda);
    mp_obj_t sda_pin = mp_pin_make_new(NULL, 1, 0, pargs);
    // create I2C object
    mp_map_elem_t elements[4] = {
        {mp_obj_new_str("scl", 3), scl_pin},
        {mp_obj_new_str("sda", 3), sda_pin},
        {mp_obj_new_str("freq", 4), mp_obj_new_int(freq)},
        {mp_obj_new_str("timeout", 7), mp_obj_new_int(100)},
    };
    _hal_i2c[next_index].key = mp_obj_new_int(id);
    _hal_i2c[next_index].value = machine_i2c_make_new(MP_OBJ_NULL, 0, 4, (const mp_obj_t*)elements);
    return _hal_i2c[next_index].value;
}


void bricks_sensors_free_i2c_peripheral(mp_int_t id) {
    mp_int_t next_index = 0;
    mp_obj_t i2c_inst = _search_i2c_periph(id, &next_index);
    if (i2c_inst != MP_OBJ_NULL) {
        _hal_i2c[next_index-1].key = MP_OBJ_NULL;
        _hal_i2c[next_index-1].value = MP_OBJ_NULL;
    }
}


STATIC const mp_rom_map_elem_t sensors_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sensors) },
    { MP_ROM_QSTR(MP_QSTR_DigitalInput), MP_ROM_PTR(&digital_input_type) },
    { MP_ROM_QSTR(MP_QSTR_LM75), MP_ROM_PTR(&lm75_type) },
    { MP_ROM_QSTR(MP_QSTR_SHT21), MP_ROM_PTR(&sht21_type) },
};
STATIC MP_DEFINE_CONST_DICT(sensors_module_globals, sensors_module_globals_table);


const mp_obj_module_t mp_module_sensors = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&sensors_module_globals,
};


#endif // MICROPY_BRICKS_SENSORS
