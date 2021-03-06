/**
 * $Id:$
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * The contents of this file may be used under the terms of either the GNU
 * General Public License Version 2 or later (the "GPL", see
 * http://www.gnu.org/licenses/gpl.html ), or the Blender License 1.0 or
 * later (the "BL", see http://www.blender.org/BL/ ) which has to be
 * bought from the Blender Foundation to become active, in which case the
 * above mentioned GPL option does not apply.
 *
 * The Original Code is Copyright (C) 2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

/*  python.c      MIXED MODEL

 * 
 *  june 99
 * Version: $Id: py_vector.c,v 1.5 2000/07/21 09:05:29 nzc Exp $
 */

#include "blender.h"
#include "py_blender.h"
#include "screen.h"

/*****************************/
/*    Vector Python Object   */
/*****************************/

PyTypeObject Vector_Type;

typedef struct {
	PyObject_VAR_HEAD
	float *vec;
	int size;
} VectorObject;

#define VectorObject_Check(v)	((v)->ob_type == &Vector_Type)

PyObject *newVectorObject(float *vec, int size) {
	VectorObject *self;
	
	self= PyObject_NEW(VectorObject, &Vector_Type);
	
	self->vec= vec;
	self->size= size;
	
	return (PyObject*) self;
}

static void Vector_dealloc(VectorObject *self) {
	PyMem_DEL(self);
}

static PyObject *Vector_getattr(VectorObject *self, char *name) {
	if (self->size==3 && ELEM3(name[0], 'x', 'y', 'z') && name[1]==0)
		return PyFloat_FromDouble(self->vec[ name[0]-'x' ]);

	PyErr_SetString(PyExc_AttributeError, name);	
	return NULL;
}

static int Vector_setattr(VectorObject *self, char *name, PyObject *v) {
	float val;
	
	Py_Try(PyArg_Parse(v, "f;Coordinates must be floats", &val));

	if (self->size==3 && ELEM3(name[0], 'x', 'y', 'z') && name[1]==0)
		self->vec[ name[0]-'x' ]= val;
	else
		return -1;
		
	return 0;
}

/* Vectors Sequence methods */

static int Vector_len(VectorObject *self) {
	return self->size;
}

static PyObject *Vector_item(VectorObject *self, int i)
{
	if (i < 0 || i >= self->size) {
		PyErr_SetString(PyExc_IndexError, "array index out of range");
		return NULL;
	}
	return Py_BuildValue("f", self->vec[i]);
}

static PyObject *Vector_slice(VectorObject *self, int begin, int end)
{
	PyObject *list;
	int count;
	
	if (begin<0) begin= 0;
	if (end>self->size) end= self->size;
	if (begin>end) begin= end;
		
	list= PyList_New(end-begin);

	for (count= begin; count<end; count++)
		PyList_SetItem(list, count-begin, PyFloat_FromDouble(self->vec[count]));
	
	return list;
}

static int Vector_ass_item(VectorObject *self, int i, PyObject *ob)
{
	float val;
	
	if (i < 0 || i >= self->size) {
		PyErr_SetString(PyExc_IndexError, "array assignment index out of range");
		return -1;
	}

	if (!PyNumber_Check(ob)) {
		PyErr_SetString(PyExc_IndexError, "vector member must be a number");
		return -1;
	}
	
	self->vec[i]= PyFloat_AsDouble(ob);
/* 	if(!PyArg_Parse(ob, "f", &)) return -1; */
	
	return 0;
}

static int Vector_ass_slice(VectorObject *self, int begin, int end, PyObject *seq)
{
	int count;
	
	if (begin<0) begin= 0;
	if (end>self->size) end= self->size;
	if (begin>end) begin= end;

	if (!PySequence_Check(seq)) {
		PyErr_SetString(PyExc_TypeError, "illegal argument type for built-in operation");
		return -1;		
	}

	if (PySequence_Length(seq)!=(end-begin)) {
		PyErr_SetString(PyExc_TypeError, "size mismatch in slice assignment");
		return -1;
	}
	
	for (count= begin; count<end; count++) {
		PyObject *ob= PySequence_GetItem(seq, count);
		if (!PyArg_Parse(ob, "f", &self->vec[count])) {
			Py_DECREF(ob);
			return -1;
		}
		Py_DECREF(ob);
	}
		
	return 0;
}

static PyObject *Vector_repr (VectorObject *self) {
	PyObject *list= Vector_slice(self, 0, self->size);
	PyObject *repr= PyObject_Repr(list);
	Py_DECREF(list);
	
	return repr;
}

static PySequenceMethods Vector_SeqMethods = {
	(inquiry)			Vector_len,			/* sq_length	*/
	(binaryfunc)		0,					/* sq_concat	*/
	(intargfunc)		0,					/* sq_repeat	*/
	(intargfunc)		Vector_item,		/* sq_item		*/
	(intintargfunc)		Vector_slice,		/* sq_slice		*/
	(intobjargproc)		Vector_ass_item,	/* sq_ass_item	*/
	(intintobjargproc)	Vector_ass_slice,	/* sq_ass_slice	*/
};

PyTypeObject Vector_Type = {
	PyObject_HEAD_INIT(NULL)
	0,								/*ob_size*/
	"Vector",						/*tp_name*/
	sizeof(VectorObject),			/*tp_basicsize*/
	0,								/*tp_itemsize*/
	/* methods */
	(destructor)	Vector_dealloc,	/*tp_dealloc*/
	(printfunc)		0,				/*tp_print*/
	(getattrfunc)	Vector_getattr,	/*tp_getattr*/
	(setattrfunc)	Vector_setattr,	/*tp_setattr*/
	0,								/*tp_compare*/
	(reprfunc)		Vector_repr,	/*tp_repr*/
	0,								/*tp_as_number*/
	&Vector_SeqMethods,				/*tp_as_sequence*/
};

void init_py_vector(void) {
	Vector_Type.ob_type = &PyType_Type;
}

