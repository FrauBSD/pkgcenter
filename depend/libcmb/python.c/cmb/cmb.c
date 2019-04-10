/*-
 * Copyright (c) 2018-2019 Devin Teske <dteske@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifdef __FBSDID
__FBSDID("$FrauBSD: pkgcenter/depend/libcmb/python.c/cmb/cmb.c 2019-04-10 07:22:17 -0700 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

#include <Python.h>
#include <cmb.h>

#include "structmember.h"

#define DEBUG	0

/* Helpers */
#if PY_MAJOR_VERSION == 2
#define KeyEq(x)	(strcmp(x, key) == 0)
#else
#define KeyEq(x)	(_PyUnicode_EqualToASCIIString(key, x))
#endif
#define CONFIG(y)	((self->config_set & CONFIG_SET_##y) == CONFIG_SET_##y)
#define LIST_APPEND(x)	(PyList_Append(list, Py_BuildValue("s", x)) != 0)
#define DPRINT(x)	fprintf(stderr, \
    "%s:%d:%s: " x "\n", __FILE__, __LINE__, __func__)
#define DPRINTF(x,...)	fprintf(stderr, \
    "%s:%d:%s: " x "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)

/* Attribute management */
#define CONFIG_SET_DEBUG		0x00000001
#define CONFIG_SET_NUL_TERMINATE	0x00000002
#define CONFIG_SET_SHOW_EMPTY		0x00000004
#define CONFIG_SET_SHOW_NUMBERS		0x00000008
#define CONFIG_SET_DELIMITER		0x00000010
#define CONFIG_SET_PREFIX		0x00000020
#define CONFIG_SET_SUFFIX		0x00000040
#define CONFIG_SET_SIZE_MIN		0x00000080
#define CONFIG_SET_SIZE_MAX		0x00000100
#define CONFIG_SET_COUNT		0x00000200
#define CONFIG_SET_START		0x00000400
#define CONFIG_SET_RESERVED1		0x00000800
#define CONFIG_SET_RESERVED2		0x00001000
#define CONFIG_SET_RESERVED3		0x00002000
#define CONFIG_SET_RESERVED4		0x00004000
#define CONFIG_SET_RESERVED5		0x00008000
#define CONFIG_SET_RESERVED6		0x00010000

/* DLL import/export */
#undef PyMODINIT_FUNC
#ifndef PyMODINIT_FUNC
#if PY_MAJOR_VERSION == 2
#define PyMODINIT_FUNC void
#else
#define PyMODINIT_FUNC PyObject *
#endif
#endif

/* Custom types */
typedef struct {
	PyObject_HEAD
	struct cmb_config *config;
	uint32_t config_set;
} PyCmbObject;

/* Function prototypes */
static void       CmbDealloc(PyCmbObject *self);
static PyObject * CmbError(PyObject *m);
#if PY_MAJOR_VERSION == 2
static PyObject * CmbGet(PyCmbObject *self, const char *key);
#else
static PyObject * CmbGetO(PyCmbObject *self, PyObject *key);
#endif
static int        CmbInit(PyCmbObject *self, PyObject *args, PyObject *kwds);
static PyObject * CmbNew(PyTypeObject *type, PyObject *args, PyObject *kwds);
#if PY_MAJOR_VERSION == 2
static int        CmbSet(PyCmbObject *self, const char *key, PyObject *value);
#else
static int        CmbSetO(PyCmbObject *self, PyObject *key, PyObject *value);
#endif
#if PY_MAJOR_VERSION >= 3
static int        cmb_traverse(PyObject *m, visitproc visit, void *arg);
static int        cmb_clear(PyObject *m);
#endif
static PyObject * pycmb(PyObject *obj, PyObject *args);
static PyObject * pycmb_callback(PyObject *obj, PyObject *args);
static PyObject * pycmb_count(PyObject *obj, PyObject *args);
static PyObject * pycmb_print(PyObject *obj, PyObject *args);
static PyObject * pycmb_version(PyObject *obj, PyObject *args);

/*
 * Globals
 */

static PyObject *py_cmb_func = NULL;

static PyTypeObject PyCmbType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "cmb.CMB",
	.tp_basicsize = sizeof(PyCmbObject),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor)CmbDealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
#if PY_MAJOR_VERSION == 2
	.tp_getattr = (getattrfunc)CmbGet,
	.tp_setattr = (setattrfunc)CmbSet,
#else
	.tp_getattro = (getattrofunc)CmbGetO,
	.tp_setattro = (setattrofunc)CmbSetO,
#endif
	.tp_doc = "cmb objects",
	.tp_init = (initproc)CmbInit,
	.tp_new = CmbNew,
};

static PyMethodDef cmbMethods[] = {
	{ "cmb", pycmb, METH_VARARGS },
	{ "cmb_callback", pycmb_callback, METH_VARARGS },
	{ "cmb_count", pycmb_count, METH_VARARGS },
	{ "cmb_print", pycmb_print, METH_VARARGS },
	{ "cmb_version", pycmb_version, METH_VARARGS },
	{ "error_out", (PyCFunction)CmbError, METH_NOARGS },
	{ NULL, NULL },
};

struct module_state {
	PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state *)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef cmbModule = {
	PyModuleDef_HEAD_INIT,
	.m_name = "cmb",
	.m_doc = "combinatorics library",
	.m_size = sizeof(PyCmbObject),
	.m_methods = cmbMethods,
	.m_traverse = cmb_traverse,
	.m_clear = cmb_clear,
};
#endif

/*
 * Type implementation
 */

static void
CmbDealloc(PyCmbObject *self)
{
	struct cmb_config *config = self->config;

#if DEBUG
	DPRINT("here");
#endif
	if (config != NULL) {
		if (config->delimiter != NULL) {
			free(config->delimiter);
			config->delimiter = NULL;
		}
		if (config->prefix != NULL) {
			free(config->prefix);
			config->prefix = NULL;
		}
		if (config->suffix != NULL) {
			free(config->suffix);
			config->suffix = NULL;
		}
		free(config);
		self->config = NULL;
	}
#if PY_MAJOR_VERSION == 2
	Py_TYPE(self)->tp_free((PyObject *)self);
#else
	Py_CLEAR(self);
#endif
}

static PyObject *
CmbNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	PyCmbObject *self;
	size_t size;

	if ((self = (PyCmbObject *)type->tp_alloc(type, 0)) == NULL)
		return NULL;
	size = sizeof(struct cmb_config);
	if ((self->config = (struct cmb_config *)calloc(1, size)) == NULL) {
		Py_DECREF(self);
		return NULL;
	}

	return ((PyObject *)self);
}

static const char *keywords[] = {
	"count",
	"debug",
	"delimiter",
	"nul_terminate",
	"prefix",
	"show_empty",
	"show_numbers",
	"size_max",
	"size_min",
	"start",
	"suffix",
	NULL
};

static int
CmbInit(PyCmbObject *self, PyObject *args, PyObject *kwds)
{
	const char *k;
	PyObject *value;
#if PY_MAJOR_VERSION >= 3
	PyObject *key;
#endif
	int i = -1;

	if (kwds == NULL || !PyDict_Check(kwds))
		return (0);

	while ((k = keywords[++i]) != NULL) {
		if ((value = PyDict_GetItemString(kwds, k)) == NULL)
			continue;
#if PY_MAJOR_VERSION == 2
		CmbSet(self, k, value);
#else
		key = Py_BuildValue("s", k);
		CmbSetO(self, key, value);
		Py_CLEAR(key);
#endif
	}

	return (0);
}

static PyObject *
CmbKeys(PyCmbObject *self)
{
	PyObject *list = PyList_New(0);

	if (CONFIG(DEBUG) && LIST_APPEND("debug"))
		goto cmb_keys_error;
	if (CONFIG(NUL_TERMINATE) && LIST_APPEND("nul_terminate"))
		goto cmb_keys_error;
	if (CONFIG(SHOW_EMPTY) && LIST_APPEND("show_empty"))
		goto cmb_keys_error;
	if (CONFIG(SHOW_NUMBERS) && LIST_APPEND("show_numbers"))
		goto cmb_keys_error;
	if (CONFIG(DELIMITER) && LIST_APPEND("delimiter"))
		goto cmb_keys_error;
	if (CONFIG(PREFIX) && LIST_APPEND("prefix"))
		goto cmb_keys_error;
	if (CONFIG(SUFFIX) && LIST_APPEND("suffix"))
		goto cmb_keys_error;
	if (CONFIG(SIZE_MIN) && LIST_APPEND("size_min"))
		goto cmb_keys_error;
	if (CONFIG(SIZE_MAX) && LIST_APPEND("size_max"))
		goto cmb_keys_error;
	if (CONFIG(COUNT) && LIST_APPEND("count"))
		goto cmb_keys_error;
	if (CONFIG(START) && LIST_APPEND("start"))
		goto cmb_keys_error;

cmb_keys_error:
	return (list);
}

static PyObject *
#if PY_MAJOR_VERSION == 2
CmbGet(PyCmbObject *self, const char *key)
#else
CmbGetO(PyCmbObject *self, PyObject *key)
#endif
{
	struct cmb_config *config = self->config;

	if (KeyEq("count"))
		return Py_BuildValue("K", config->count);
	else if (KeyEq("debug"))
		return Py_BuildValue("b", (config->options &
		    CMB_OPT_DEBUG) == 0 ? FALSE : TRUE);
	else if (KeyEq("delimiter"))
		return Py_BuildValue("s", config->delimiter);
	else if (KeyEq("keys"))
		return CmbKeys(self);
	else if (KeyEq("nul_terminate"))
		return Py_BuildValue("b", (config->options &
		    CMB_OPT_NULPRINT) == 0 ? FALSE : TRUE);
	else if (KeyEq("prefix"))
		return Py_BuildValue("s", config->prefix);
	else if (KeyEq("show_empty"))
		return Py_BuildValue("b", (config->options &
		    CMB_OPT_EMPTY) == 0 ? FALSE : TRUE);
	else if (KeyEq("show_numbers"))
		return Py_BuildValue("b", (config->options &
		    CMB_OPT_NUMBERS) == 0 ? FALSE : TRUE);
	else if (KeyEq("size_max"))
		return Py_BuildValue("I", config->size_max);
	else if (KeyEq("size_min"))
		return Py_BuildValue("I", config->size_min);
	else if (KeyEq("start"))
		return Py_BuildValue("K", config->start);
	else if (KeyEq("suffix"))
		return Py_BuildValue("s", config->suffix);

	PyErr_Format(PyExc_AttributeError,
	    "'%.200s' object has no attribute '%s'",
	    Py_TYPE(self)->tp_name,
#if PY_MAJOR_VERSION == 2
	    key
#else
	    PyUnicode_AsUTF8(key)
#endif
	);

	return (NULL);
}

static int
#if PY_MAJOR_VERSION == 2
CmbSet(PyCmbObject *self, const char *key, PyObject *value)
#else
CmbSetO(PyCmbObject *self, PyObject *key, PyObject *value)
#endif
{
	struct cmb_config *config = self->config;
	const char *tmp;
	size_t len;

#if PY_MAJOR_VERSION >= 3
	if (!PyUnicode_Check(key)) {
		PyErr_Format(PyExc_TypeError,
			"attribute name must be string, not '%.200s'",
			Py_TYPE(key)->tp_name);
		return (-1);
	}
#endif

	if (key == NULL) {
		PyErr_SetString(PyExc_TypeError, "Attribute name required!");
		return (-1);
	}
	if (KeyEq("keys")) {
		self->config_set = 0;
		bzero(self->config, sizeof(struct cmb_config));
	} else if (KeyEq("count")) {
		if (PyNumber_Check(value)) {
			config->count = (uint64_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_COUNT;
		} else {
			config->count = 0;
			self->config_set &= ~CONFIG_SET_COUNT;
		}
	} else if (KeyEq("debug")) {
		if (PyNumber_Check(value)) {
			if (PyLong_AsUnsignedLong(value) == 0) {
				config->options &= ~CMB_OPT_DEBUG;
			} else {
				config->options |= CMB_OPT_DEBUG;
			}
			self->config_set |= CONFIG_SET_DEBUG;
		} else {
			config->options &= ~CMB_OPT_DEBUG;
			self->config_set &= ~CONFIG_SET_DEBUG;
		}
	} else if (KeyEq("delimiter")) {
		self->config_set |= CONFIG_SET_DELIMITER;
		if (config->delimiter != NULL) {
			free(config->delimiter);
			config->delimiter = NULL;
		}
#if PY_MAJOR_VERSION == 2
		if (PyString_Check(value) &&
		    (tmp = PyString_AsString(value)) != NULL) {
#else
		if (PyUnicode_Check(value) &&
		    (tmp = PyUnicode_AsUTF8(value)) != NULL) {
#endif
			len = strlen(tmp) + 1;
			if ((config->delimiter = (char *)malloc(len)) != NULL) {
				memcpy(config->delimiter, tmp, len);
				self->config_set |= CONFIG_SET_DELIMITER;
			}
		}
	} else if (KeyEq("nul_terminate")) {
		if (PyNumber_Check(value)) {
			if (PyLong_AsUnsignedLong(value) == 0) {
				config->options &= ~CMB_OPT_NULPRINT;
			} else {
				config->options |= CMB_OPT_NULPRINT;
			}
			self->config_set |= CONFIG_SET_NUL_TERMINATE;
		} else {
			config->options &= ~CMB_OPT_NULPRINT;
			self->config_set &= ~CONFIG_SET_NUL_TERMINATE;
		}
	} else if (KeyEq("prefix")) {
		self->config_set |= CONFIG_SET_PREFIX;
		if (config->prefix != NULL) {
			free(config->prefix);
			config->prefix = NULL;
		}
#if PY_MAJOR_VERSION == 2
		if (PyString_Check(value) &&
		    (tmp = PyString_AsString(value)) != NULL) {
#else
		if (PyUnicode_Check(value) &&
		    (tmp = PyUnicode_AsUTF8(value)) != NULL) {
#endif
			len = strlen(tmp) + 1;
			if ((config->prefix = (char *)malloc(len)) != NULL) {
				memcpy(config->prefix, tmp, len);
				self->config_set |= CONFIG_SET_PREFIX;
			}
		}
	} else if (KeyEq("show_empty")) {
		if (PyNumber_Check(value)) {
			if (PyLong_AsUnsignedLong(value) == 0) {
				config->options &= ~CMB_OPT_EMPTY;
			} else {
				config->options |= CMB_OPT_EMPTY;
			}
			self->config_set |= CONFIG_SET_SHOW_EMPTY;
		} else {
			config->options &= ~CMB_OPT_EMPTY;
			self->config_set &= ~CONFIG_SET_SHOW_EMPTY;
		}
	} else if (KeyEq("show_numbers")) {
		if (PyNumber_Check(value)) {
			if (PyLong_AsUnsignedLong(value) == 0) {
				config->options &= ~CMB_OPT_NUMBERS;
			} else {
				config->options |= CMB_OPT_NUMBERS;
			}
			self->config_set |= CONFIG_SET_SHOW_NUMBERS;
		} else {
			config->options &= ~CMB_OPT_NUMBERS;
			self->config_set &= ~CONFIG_SET_SHOW_NUMBERS;
		}
	} else if (KeyEq("size_max")) {
		if (PyNumber_Check(value)) {
			config->size_max =
			    (uint32_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_SIZE_MAX;
		} else {
			config->size_max = 0;
			self->config_set &= ~CONFIG_SET_SIZE_MAX;
		}
	} else if (KeyEq("size_min")) {
		if (PyNumber_Check(value)) {
			config->size_min =
			    (uint32_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_SIZE_MIN;
		} else {
			config->size_min = 0;
			self->config_set &= ~CONFIG_SET_SIZE_MIN;
		}
	} else if (KeyEq("start")) {
		if (PyNumber_Check(value)) {
			config->start = (uint64_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_START;
		} else {
			config->start = 0;
			self->config_set &= ~CONFIG_SET_START;
		}
	} else if (KeyEq("suffix")) {
		self->config_set |= CONFIG_SET_SUFFIX;
		if (config->suffix != NULL) {
			free(config->suffix);
			config->suffix = NULL;
		}
#if PY_MAJOR_VERSION == 2
		if (PyString_Check(value) &&
		    (tmp = PyString_AsString(value)) != NULL) {
#else
		if (PyUnicode_Check(value) &&
		    (tmp = PyUnicode_AsUTF8(value)) != NULL) {
#endif
			len = strlen(tmp) + 1;
			if ((config->suffix = (char *)malloc(len)) != NULL) {
				memcpy(config->suffix, tmp, len);
				self->config_set |= CONFIG_SET_SUFFIX;
			}
		}
	} else {
		PyErr_Format(PyExc_AttributeError,
		    "'%.200s' object has no attribute '%s'",
		    Py_TYPE(self)->tp_name,
#if PY_MAJOR_VERSION == 2
		    key
#else
		    PyUnicode_AsUTF8(key)
#endif
		);
		return (-1);
	}

	return (0);
}

/*
 * Module implementation
 */

static PyObject *
CmbError(PyObject *m)
{
    struct module_state *st = GETSTATE(m);

    PyErr_SetString(st->error, "something bad happened");
    return (NULL);
}

static PyObject *
pycmb(PyObject *obj, PyObject *args)
{
	int i = 0;
	int res = 0;
	size_t len;
	ssize_t size = 0;
	PyObject *list;
	PyObject *pysize;
	PyObject *pListItem;
	PyCmbObject *self;
	const char *tmp;
	char **v = NULL;

	/* Parse and type-check arguments */
	if (!PyArg_ParseTuple(args, "OiO", &self, &pysize, &list))
		return NULL;
	if (!PyList_Check(list))
		goto pycmb_error;

	/* Allocate memory */
	size = PyList_Size(list);
	v = (char **)malloc(sizeof(char *) * size);
	for (i = 0; i < size; i++) {
		pListItem = PyList_GetItem(list, i);
#if PY_MAJOR_VERSION == 2
		if ((tmp = PyString_AsString(pListItem)) != NULL) {
#else
		if ((tmp = PyUnicode_AsUTF8(pListItem)) != NULL) {
#endif
			len = strlen(tmp) + 1;
			v[i] = (char *)malloc(len);
			memcpy(v[i], tmp, len);
		} else
			v[i] = (char *)calloc(1, 1);
	}

	res = cmb(self->config, (uint32_t)size, (char **)v);

	/* Free memory */
	for (i = 0; i < size; i++)
		free(v[i]);
	free(v);

pycmb_error:
	return (Py_BuildValue("i", res));
}

static uint32_t stub_cmb_func_list_size = 0;
static PyObject *stub_cmb_func_list = NULL;
static PyObject *stub_cmb_func_args = NULL;

static int
stub_cmb_func(struct cmb_config *config, uint64_t seq, uint32_t nitems,
	char *items[])
{
	PyObject *resObj;
	int res = 0;
	uint32_t i;

	/* Resize list arguments if necessary */
	if (stub_cmb_func_list_size != nitems) {
		PyObject *old = stub_cmb_func_list;
		stub_cmb_func_list =
		    PyList_New(stub_cmb_func_list_size = nitems);
		PyTuple_SET_ITEM(stub_cmb_func_args, 0, stub_cmb_func_list);
		Py_CLEAR(old);
	}

	for (i = 0; i < nitems; i++) {
		PyList_SET_ITEM(stub_cmb_func_list, i, (PyObject *)items[i]);
#if DEBUG
		DPRINTF("items[%i].ob_refcnt = %zi", i,
		    ((PyObject *)items[i])->ob_refcnt);
#endif
	}
#if DEBUG
	DPRINT("calling python function");
#endif
	resObj = PyObject_CallObject(py_cmb_func, stub_cmb_func_args);
	if (resObj == NULL) {
#if DEBUG
		DPRINT("something went wrong");
#endif
		PyErr_Print();
	}
	res = (int)PyLong_AsLong(resObj);
#if DEBUG
	DPRINTF("result: %i", res);
	DPRINTF("py_cmb_func.ob_refcnt = %zi", py_cmb_func->ob_refcnt);
#endif
	Py_CLEAR(resObj);

	return (res);
}

static PyObject *
pycmb_callback(PyObject *obj, PyObject *args)
{
	int res = 0;
	uint32_t nitems, i;
	ssize_t size;
	PyCmbObject *self;
	PyObject *func;
	PyObject *list;
	PyObject **v;

	/* Parse and type-check arguments */
	if (!PyArg_ParseTuple(args, "OIOO", &self, &nitems, &list, &func))
		return NULL;
	if (!PyList_Check(list))
		goto pycmb_callback_error;
	if (!PyCallable_Check(func)) {
		PyErr_SetString(PyExc_TypeError, "Need a callable object!");
		goto pycmb_callback_error;
	}

	/* Allocate memory */
	Py_XINCREF(self);
	Py_XINCREF(list);
	Py_XDECREF(py_cmb_func);
	Py_XINCREF(py_cmb_func = func);
#if DEBUG
	DPRINT("initializing");
	DPRINTF("self.ob_refcnt = %zi", self->ob_refcnt);
	DPRINTF("args.ob_refcnt = %zi", args->ob_refcnt);
#endif
	size = PyList_Size(list);
	v = (PyObject **)malloc(sizeof(PyObject *) * size);
	for (i = 0; i < (uint32_t)size; i++) {
		v[i] = PyList_GetItem(list, i); /* borrowed ref */
#if DEBUG
		DPRINTF("v[%i].ob_refcnt = %zi", i, v[i]->ob_refcnt);
#endif
	}
#if DEBUG
	DPRINTF("py_cmb_func.ob_refcnt = %zi", py_cmb_func->ob_refcnt);
#endif

	self->config->action = stub_cmb_func;
	stub_cmb_func_list_size = 0;
	stub_cmb_func_list = PyList_New(stub_cmb_func_list_size);
#if DEBUG
	DPRINTF("stub_cmb_func_list.ob_refcnt = %zi",
	    stub_cmb_func_list->ob_refcnt);
#endif
	stub_cmb_func_args = Py_BuildValue("(O)", stub_cmb_func_list);
#if DEBUG
	DPRINTF("stub_cmb_func_list.ob_refcnt = %zi",
	    stub_cmb_func_list->ob_refcnt);
#endif
	Py_DECREF(stub_cmb_func_list);
#if DEBUG
	DPRINTF("stub_cmb_func_list.ob_refcnt = %zi",
	    stub_cmb_func_list->ob_refcnt);
	DPRINT("initialized");
#endif

	res = cmb(self->config, nitems, (char **)v);

#if DEBUG
	DPRINT("cleaning up");
	DPRINTF("self.ob_refcnt = %zi", self->ob_refcnt);
	DPRINTF("args.ob_refcnt = %zi", args->ob_refcnt);
	for (i = 0; i < (uint32_t)size; i++)
		DPRINTF("v[%i].ob_refcnt = %zi", i, v[i]->ob_refcnt);
	DPRINTF("py_cmb_func.ob_refcnt = %zi", py_cmb_func->ob_refcnt);
	DPRINTF("stub_cmb_func_list.ob_refcnt = %zi",
	    stub_cmb_func_list->ob_refcnt);
#endif

	/* Free memory */
	Py_DECREF(args);
	Py_DECREF(py_cmb_func);
	Py_DECREF(stub_cmb_func_args);
	Py_DECREF(stub_cmb_func_list);
	stub_cmb_func_args = NULL;
	stub_cmb_func_list = NULL;
	stub_cmb_func_list_size = 0;
	free(v);

	usleep(5000);

pycmb_callback_error:
	return (Py_BuildValue("i", res));
}

static PyObject *
pycmb_count(PyObject *obj, PyObject *args)
{
	uint64_t i = 0;
	PyCmbObject *self;
	PyObject *nitems;

	/* Parse and type-check arguments */
	if (!PyArg_ParseTuple(args, "OO", &self, &nitems))
		return NULL;
	if (!PyNumber_Check(nitems))
		goto pycmb_count_error;

	i = cmb_count(self->config, (uint32_t)PyLong_AsUnsignedLong(nitems));

pycmb_count_error:
	return (Py_BuildValue("K", i));
}

static PyObject *
pycmb_print(PyObject *obj, PyObject *args)
{
	PyObject *list, *pListItem;
	PyCmbObject *self;
	int i;
	int res = 0;
	uint32_t nitems;
	uint64_t seq;
	size_t len;
	ssize_t size;
	char **v;
	const char *tmp;

	if (!PyArg_ParseTuple(args, "OKIO", &self, &seq, &nitems, &list))
		goto pycmb_print_error;
	if (!PyList_Check(list))
		goto pycmb_print_error;

	/* Allocate memory */
	size = PyList_Size(list);
	v = (char **)malloc(sizeof(char *) * size);
	for (i = 0; i < size; i++) {
		pListItem = PyList_GetItem(list, i);
		if (pListItem == NULL)
			continue;
#if PY_MAJOR_VERSION == 2
		if ((tmp = PyString_AsString(pListItem)) != NULL) {
#else
		if ((tmp = PyUnicode_AsUTF8(pListItem)) != NULL) {
#endif
			len = strlen(tmp) + 1;
			v[i] = (char *)malloc(len);
			memcpy(v[i], tmp, len);
		} else
			v[i] = (char *)calloc(1, 1);
	}

	res = cmb_print(self->config, seq, nitems, v);

	/* Free memory */
	for (i = 0; i < size; i++)
		free(v[i]);
	free(v);

pycmb_print_error:
	return (Py_BuildValue("i", res));
}

static PyObject *
pycmb_version(PyObject *obj, PyObject *args)
{
	PyObject *pytype;
	int type = 0;
#if PY_MAJOR_VERSION == 2
	const char *opt = "O";
#else
	const char *opt = "|O";
#endif

	if (PyArg_ParseTuple(args, opt, &pytype) && PyNumber_Check(pytype))
		type = (int)PyLong_AsLong(pytype);
	return (Py_BuildValue("s", cmb_version(type)));
}

#if PY_MAJOR_VERSION >= 3
static int
cmb_traverse(PyObject *m, visitproc visit, void *arg)
{
	struct module_state *st;

	st = GETSTATE(m);
	if (st != NULL)
		Py_VISIT(st->error);

	return (0);
}

static int
cmb_clear(PyObject *m)
{
	struct module_state *st;

	st = GETSTATE(m);
	if (st != NULL)
		Py_CLEAR(st->error);

	return (0);
}
#endif /* PY_MAJOR_VERSION >= 3 */

PyMODINIT_FUNC
#if PY_MAJOR_VERSION == 2
initcmb(void)
#else
PyInit_cmb(void)
#endif
{
	PyObject *m = NULL;
	struct module_state *st;

	if (PyType_Ready(&PyCmbType) < 0)
		goto initcmb_error;

#if PY_MAJOR_VERSION == 2
	m = Py_InitModule3("cmb", cmbMethods, "Combinatorics module");
#else
	m = PyModule_Create(&cmbModule);
#endif
	if (m == NULL)
		goto initcmb_error;

	st = GETSTATE(m);
	if (st == NULL)
		goto initcmb_error;
	st->error = PyErr_NewException("cmb.Error", NULL, NULL);
	if (st->error == NULL) {
		Py_DECREF(m);
		m = NULL;
		goto initcmb_error;
	}

	Py_INCREF(&PyCmbType);
	PyModule_AddObject(m, "CMB", (PyObject *)&PyCmbType); /* steals ref */

initcmb_error:
	PyErr_Print();
#if PY_MAJOR_VERSION >= 3
	return (m);
#else
	return;
#endif
}
