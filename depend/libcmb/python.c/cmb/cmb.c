/*-
 * Copyright (c) 2018 Devin Teske <dteske@FreeBSD.org>
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
__FBSDID("$FrauBSD: pkgcenter/depend/libcmb/python.c/cmb/cmb.c 2018-12-31 23:23:46 -0800 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

#include <Python.h>
#include <cmb.h>
#include "structmember.h"

/* Helpers */
#define KeyEq(x)	(strcmp(x, key) == 0)
#define CONFIG(y)	((self->config_set & CONFIG_SET_##y) == CONFIG_SET_##y)
#define LIST_APPEND(x)	(PyList_Append(list, Py_BuildValue("s", x)) != 0)

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
#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

/* Custom types */
typedef struct {
	PyObject_HEAD
	struct cmb_config *config;
	uint32_t config_set;
} PyCmbObject;

/* Function prototypes */
static PyObject * CmbNew(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int        CmbInit(PyCmbObject *self, PyObject *args, PyObject *kwds);
static PyObject * CmbGet(PyCmbObject *self, char *name);
static int        CmbSet(PyCmbObject *self, char *name, PyObject *value);
static void       CmbDealloc(PyCmbObject *self);

/*
 * Globals
 */

static PyObject *py_cmb_func = NULL;

static PyTypeObject PyCmbType = {
	PyObject_HEAD_INIT(NULL)
	0,                      /* ob_size */
	"cmb.CMB",              /* tp_name */
	sizeof(PyCmbObject),    /* tp_basicsize */
	0,                      /* tp_itemsize */
	(destructor)CmbDealloc, /* tp_dealloc */
	NULL,                   /* tp_print */
	(getattrfunc)CmbGet,    /* tp_getattr */
	(setattrfunc)CmbSet,    /* tp_setattr */
	NULL,                   /* tp_compare */
	NULL,                   /* tp_repr */
	NULL,                   /* tp_as_number */
	NULL,                   /* tp_as_sequence */
	NULL,                   /* tp_as_mapping */
	NULL,                   /* tp_hash  */
	NULL,                   /* tp_call */
	NULL,                   /* tp_str */
	NULL,                   /* tp_getattro */
	NULL,                   /* tp_setattro */
	NULL,                   /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	                        /* tp_flags */
	"cmb objects",          /* tp_doc */
	NULL,                   /* tp_traverse */
	NULL,                   /* tp_clear */
	NULL,                   /* tp_richcompare */
	0,                      /* tp_weaklistoffset */
	NULL,                   /* tp_iter */
	NULL,                   /* tp_iternext */
	NULL,                   /* tp_methods */
	NULL,                   /* tp_members */
	NULL,                   /* tp_getset */
	NULL,                   /* tp_base */
	NULL,                   /* tp_dict */
	NULL,                   /* tp_descr_get */
	NULL,                   /* tp_descr_set */
	0,                      /* tp_dictoffset */
	(initproc)CmbInit,      /* tp_init */
	NULL,                   /* tp_alloc */
	CmbNew,                 /* tp_new */
};

/*
 * Type implementation
 */

static void
CmbDealloc(PyCmbObject *self)
{
	struct cmb_config *config = self->config;

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
	self->ob_type->tp_free((PyObject *)self);
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

static char *keywords[] = {
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
	char **k;
	PyObject *value;

	if (kwds != NULL && PyDict_Check(kwds)) {
		k = keywords - 1;
		while (*++k != NULL)
			if ((value = PyDict_GetItemString(kwds, *k)) != NULL)
				CmbSet(self, *k, value);
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
CmbGet(PyCmbObject *self, char *key)
{
	struct cmb_config *config = self->config;

	if (KeyEq("count"))
		return Py_BuildValue("K", config->count);
	else if (KeyEq("debug"))
		return Py_BuildValue("b", config->debug);
	else if (KeyEq("delimiter"))
		return Py_BuildValue("s", config->delimiter);
	else if (KeyEq("keys"))
		return CmbKeys(self);
	else if (KeyEq("nul_terminate"))
		return Py_BuildValue("b", config->nul_terminate);
	else if (KeyEq("prefix"))
		return Py_BuildValue("s", config->prefix);
	else if (KeyEq("show_empty"))
		return Py_BuildValue("b", config->show_empty);
	else if (KeyEq("show_numbers"))
		return Py_BuildValue("b", config->show_numbers);
	else if (KeyEq("size_max"))
		return Py_BuildValue("I", config->size_max);
	else if (KeyEq("size_min"))
		return Py_BuildValue("I", config->size_min);
	else if (KeyEq("start"))
		return Py_BuildValue("K", config->start);
	else if (KeyEq("suffix"))
		return Py_BuildValue("s", config->suffix);

	Py_INCREF(Py_None);
	return (Py_None);
}

static int
CmbSet(PyCmbObject *self, char *key, PyObject *value)
{
	struct cmb_config *config = self->config;
	char *tmp;
	size_t len;

	if (KeyEq("keys")) {
		self->config_set = 0;
		bzero(self->config, sizeof(struct cmb_config));
	} else if (KeyEq("count")) {
		if (PyNumber_Check(value)) {
			config->count = (uint64_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_COUNT;
		} else {
			config->count = 0;
			self->config_set ^= CONFIG_SET_COUNT;
		}
	} else if (KeyEq("debug")) {
		if (PyNumber_Check(value)) {
			config->debug = (uint8_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_DEBUG;
		} else {
			config->debug = 0;
			self->config_set ^= CONFIG_SET_DEBUG;
		}
	} else if (KeyEq("delimiter")) {
		self->config_set ^= CONFIG_SET_DELIMITER;
		if (config->delimiter != NULL) {
			free(config->delimiter);
			config->delimiter = NULL;
		}
		if (PyString_Check(value) &&
		    (tmp = PyString_AsString(value)) != NULL) {
			len = strlen(tmp) + 1;
			if ((config->delimiter = (char *)malloc(len)) != NULL) {
				memcpy(config->delimiter, tmp, len);
				self->config_set |= CONFIG_SET_DELIMITER;
			}
		}
	} else if (KeyEq("nul_terminate")) {
		if (PyNumber_Check(value)) {
			config->nul_terminate =
			    (uint8_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_NUL_TERMINATE;
		} else {
			config->nul_terminate = 0;
			self->config_set ^= CONFIG_SET_NUL_TERMINATE;
		}
	} else if (KeyEq("prefix")) {
		self->config_set ^= CONFIG_SET_PREFIX;
		if (config->prefix != NULL) {
			free(config->prefix);
			config->prefix = NULL;
		}
		if (PyString_Check(value) &&
		    (tmp = PyString_AsString(value)) != NULL) {
			len = strlen(tmp) + 1;
			if ((config->prefix = (char *)malloc(len)) != NULL) {
				memcpy(config->prefix, tmp, len);
				self->config_set |= CONFIG_SET_PREFIX;
			}
		}
	} else if (KeyEq("show_empty")) {
		if (PyNumber_Check(value)) {
			config->show_empty =
			    (uint8_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_SHOW_EMPTY;
		} else {
			config->show_empty = 0;
			self->config_set ^= CONFIG_SET_SHOW_EMPTY;
		}
	} else if (KeyEq("show_numbers")) {
		if (PyNumber_Check(value)) {
			config->show_numbers =
			    (uint8_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_SHOW_NUMBERS;
		} else {
			config->show_numbers = 0;
			self->config_set ^= CONFIG_SET_SHOW_NUMBERS;
		}
	} else if (KeyEq("size_max")) {
		if (PyNumber_Check(value)) {
			config->size_max =
			    (uint32_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_SIZE_MAX;
		} else {
			config->size_max = 0;
			self->config_set ^= CONFIG_SET_SIZE_MAX;
		}
	} else if (KeyEq("size_min")) {
		if (PyNumber_Check(value)) {
			config->size_min =
			    (uint32_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_SIZE_MIN;
		} else {
			config->size_min = 0;
			self->config_set ^= CONFIG_SET_SIZE_MIN;
		}
	} else if (KeyEq("start")) {
		if (PyNumber_Check(value)) {
			config->start = (uint64_t)PyLong_AsUnsignedLong(value);
			self->config_set |= CONFIG_SET_START;
		} else {
			config->start = 0;
			self->config_set ^= CONFIG_SET_START;
		}
	} else if (KeyEq("suffix")) {
		self->config_set ^= CONFIG_SET_SUFFIX;
		if (config->suffix != NULL) {
			free(config->suffix);
			config->suffix = NULL;
		}
		if (PyString_Check(value) &&
		    (tmp = PyString_AsString(value)) != NULL) {
			len = strlen(tmp) + 1;
			if ((config->suffix = (char *)malloc(len)) != NULL) {
				memcpy(config->suffix, tmp, len);
				self->config_set |= CONFIG_SET_SUFFIX;
			}
		}
	}

	return (0);
}

/*
 * Module implementation
 */

static PyObject *
pycmb(PyObject *obj, PyObject *args)
{
	int i = 0;
	int res = 0;
	int size = 0;
	size_t len;
	PyObject *list;
	PyObject *pysize;
	PyObject *pListItem;
	PyCmbObject *self;
	char *tmp;
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
		if ((tmp = PyString_AsString(pListItem)) != NULL) {
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

static int
stub_cmb_func(struct cmb_config *config, uint64_t seq, uint32_t nitems,
	char *items[])
{
	PyObject *list = PyList_New(0);
	PyObject *args = Py_BuildValue("(O)", list);
	PyObject *resObj;
	int res = 0;
	uint32_t i;

	if (args == NULL) {
		if (list != NULL)
			Py_DECREF(list);
		return (1);
	}

	for (i = 0; i < nitems; i++)
		PyList_Append(list, (PyObject *)items[i]);

	resObj = PyObject_CallObject(py_cmb_func, args);
	Py_DECREF(args);
	Py_DECREF(list);

	res = (int)PyLong_AsLong(resObj);
	if (resObj != NULL)
		Py_DECREF(resObj);

	return (res);
}

static PyObject *
pycmb_callback(PyObject *obj, PyObject *args)
{
	int res = 0;
	int size;
	uint32_t nitems, i;
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
	py_cmb_func = func;
	size = PyList_Size(list);
	v = (PyObject **)malloc(sizeof(PyObject *) * size);
	for (i = 0; i < (uint32_t)size; i++)
		v[i] = PyList_GetItem(list, i); /* borrowed ref */

	self->config->action = stub_cmb_func;
	res = cmb(self->config, nitems, (char **)v);

	/* Free memory */
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
	int size;
	uint32_t nitems;
	uint64_t seq;
	size_t len;
	char **v, *tmp;

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
		if ((tmp = PyString_AsString(pListItem)) != NULL) {
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

	if (PyArg_ParseTuple(args, "O", &pytype) && PyNumber_Check(pytype))
		type = (int)PyLong_AsLong(pytype);
	return (Py_BuildValue("s", cmb_version(type)));
}

static PyMethodDef cmbMethods[] = {
	{ "cmb", pycmb, METH_VARARGS },
	{ "cmb_callback", pycmb_callback, METH_VARARGS },
	{ "cmb_count", pycmb_count, METH_VARARGS },
	{ "cmb_print", pycmb_print, METH_VARARGS },
	{ "cmb_version", pycmb_version, METH_VARARGS },
	{ NULL, NULL },
};

PyMODINIT_FUNC
initcmb(void)
{
	PyObject *m;

	if (PyType_Ready(&PyCmbType) < 0)
		return;

	m = Py_InitModule3("cmb", cmbMethods, "Combinatorics module");
	if (m == NULL)
		return;

	Py_INCREF(&PyCmbType);
	PyModule_AddObject(m, "CMB", (PyObject *)&PyCmbType); /* steals ref */
}
