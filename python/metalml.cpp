#include <Python.h>
#include <memory>
#include "../src/includes/metal-ml.h"

#define require_or_set_err(expr, err, retval) \
if ( !(expr) ) { \
PyErr_SetString(err, #expr); \
return retval; \
}

// Global metal device
static std::shared_ptr<MetalDevice> __METAL_DEVICE = 0;

template<typename K, typename V>
struct _metal_map_object {
    using key = mtl_map<K, V>::key;
    using value = mtl_map<K, V>::value;
    PyObject_HEAD
    mtl_map<K, V> * _map;
};

template<typename K, typename V>
using MetalMapObject = struct _metal_map_object<K,V>;

template<typename K, typename V>
static int MapInit(MetalMapObject<K,V> * self, PyObject * args, PyObject * kwds) {
    size_t map_size;
    if (!PyArg_ParseTuple(args, "n", &map_size)) {
        return -1;
    }
    self->_map = new mtl_map<K, V>(__METAL_DEVICE);
    self->_map->reserve(map_size);
    self->_map->init();
    return 0;
};

template<typename K, typename V>
static void MapDealloc(MetalMapObject<K, V> * self) {
    delete self->_map;
    Py_TYPE(self)->tp_free((PyObject *) self);
};

template<typename K, typename V>
static PyObject * MapInsertMulti(MetalMapObject<K, V> *self, PyObject *args) {
    Py_buffer k_buffer;
    Py_buffer v_buffer;
    
    PyObject * key_exporter;
    PyObject * value_exporter;
    
    using key = MetalMapObject<K, V>::key;
    using value = MetalMapObject<K, V>::value;
    
    require_or_set_err( PyArg_ParseTuple(args, "OO", &key_exporter, &value_exporter) >= 0 , PyExc_RuntimeError, nullptr);
    
    // Check buffer protocol compatibility
    require_or_set_err(PyObject_CheckBuffer(key_exporter), PyExc_RuntimeError, nullptr );
    require_or_set_err(PyObject_CheckBuffer(value_exporter), PyExc_RuntimeError, nullptr );
    
    
    // Get buffers
    require_or_set_err(!PyObject_GetBuffer(key_exporter, &k_buffer, PyBUF_SIMPLE), PyExc_RuntimeError,nullptr);
    require_or_set_err(!PyObject_GetBuffer(value_exporter, &v_buffer, PyBUF_SIMPLE), PyExc_RuntimeError,nullptr);
    
    // Check data types
    require_or_set_err((v_buffer.itemsize = sizeof(value)),PyExc_RuntimeError, nullptr);
    require_or_set_err((k_buffer.itemsize = sizeof(key)),PyExc_RuntimeError, nullptr);

    // Check one key = one value
    require_or_set_err((v_buffer.len / sizeof(value)  == k_buffer.len / sizeof(key)), PyExc_RuntimeError, nullptr);
    
    size_t entry_n = v_buffer.len / sizeof(value);

    key *k_buf = (key *) k_buffer.buf;
    require_or_set_err(k_buf, PyExc_RuntimeError, nullptr);
    value *v_buf = (value *) v_buffer.buf;
    require_or_set_err(v_buf, PyExc_RuntimeError, nullptr);

    pair<key, value> *entries = new pair<key, value>[entry_n];
    for (size_t i = 0; i < entry_n; i++) {
        entries[i] = pair<K, V>(k_buf[i], v_buf[i]);
    }
    
    require_or_set_err(self->_map, PyExc_RuntimeError, nullptr);
    self->_map->insert_multi(entries, entries + entry_n);

    PyBuffer_Release(&k_buffer);
    PyBuffer_Release(&v_buffer);

    delete [] entries;
    
    Py_INCREF(Py_None);
    return Py_None;
};


template<typename K, typename V>
static PyMethodDef map_methods[] = {
    {"insert", (PyCFunction) MapInsertMulti<K,V>, METH_VARARGS, "Insert key-value"},
    {NULL}
};

template<typename K, typename V>
static PyTypeObject MetalMapType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pymetal.Map",
    .tp_doc = PyDoc_STR("A Metal accelerated hashmap."),
    .tp_basicsize = sizeof(MetalMapObject<K,V>),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) MapInit<K,V>,
    .tp_dealloc = (destructor) MapDealloc<K,V>,
    .tp_methods = map_methods<K,V>
};


static int metal_module_exec(PyObject *m){
    if (PyType_Ready(&MetalMapType<uint32_t, uint32_t>) < 0) {
        return -1;
    }
    if (PyModule_AddObject(m, "Map", (PyObject *) &MetalMapType<uint32_t, uint32_t>) < 0) {
        return -1;
    }
    __METAL_DEVICE = std::make_shared<MetalDevice>();
    __METAL_DEVICE->init_lib("../libs/metalml-metal.metallib");
    return 0;
};

void metal_module_free(void *){
    __METAL_DEVICE.reset();
}


static PyModuleDef_Slot metal_slots[] = {
    {Py_mod_exec, (void *) metal_module_exec},
    {0, NULL}
};

static PyMethodDef metal_methods[] = {
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef metal_module = {
    .m_name = "pymetal",
    .m_base = PyModuleDef_HEAD_INIT,
    .m_methods = metal_methods,
    .m_slots = metal_slots,
    .m_free = metal_module_free
};

PyMODINIT_FUNC
PyInit_metal(void) {
    return PyModuleDef_Init(&metal_module);
}
