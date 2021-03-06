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
 * Version: $Id: py_matrix.c,v 1.3 2000/07/21 09:05:29 nzc Exp $
 */

#include "blender.h"
#include "py_blender.h"
#include "screen.h"

/*****************************/
/*    Matrix Python Object   */
/*****************************/

PyTypeObject Matrix_Type;

typedef struct {
	PyObject_VAR_HEAD
	PyObject *rows[4];
	float *mat;
} MatrixObject;


PyObject *newMatrixObject(float *mat) {
	MatrixObject *self;
	
	self= PyObject_NEW(MatrixObject, &Matrix_Type);
	self->mat= mat;
		
	Py_Try(self->rows[0]= newVectorObject(self->mat, 4));
	Py_Try(self->rows[1]= newVectorObject(self->mat+4, 4));
	Py_Try(self->rows[2]= newVectorObject(self->mat+8, 4));
	Py_Try(self->rows[3]= newVectorObject(self->mat+12, 4));
	
	return (PyObject*) self;
}

static void Matrix_dealloc(MatrixObject *self) {
	Py_DECREF(self->rows[0]);
	Py_DECREF(self->rows[1]);
	Py_DECREF(self->rows[2]);
	Py_DECREF(self->rows[3]);

	PyMem_DEL(self);
}

static PyObject *Matrix_getattr(MatrixObject *self, char *name) {
	PyObject *list;
	float val[3];
	
	if (strcmp(name, "rot")==0) {
		float mat3[3][3];

		Mat3CpyMat4(mat3, self->mat);
		Mat3ToEul(mat3, val);

	} else if (strcmp(name, "size")==0) {
		Mat4ToSize(self->mat, val);

	} else if (strcmp(name, "loc")==0) {
		VECCOPY(val, (self->mat+12));

	} else {
		PyErr_SetString(PyExc_AttributeError, name);
		return NULL;
	}

	list= PyList_New(3);
	PyList_SetItem(list, 0, PyFloat_FromDouble(val[0]));
	PyList_SetItem(list, 1, PyFloat_FromDouble(val[1]));
	PyList_SetItem(list, 2, PyFloat_FromDouble(val[2]));
		
	return list;
}

static int Matrix_setattr(MatrixObject *self, char *name, PyObject *v) {
	return -1;
}

static PyObject *Matrix_repr (MatrixObject *self) {
	PyObject *list= PyList_New(4);
	PyObject *repr;
	
	PyList_SetItem(list, 0, self->rows[0]);
	PyList_SetItem(list, 1, self->rows[1]);
	PyList_SetItem(list, 2, self->rows[2]);
	PyList_SetItem(list, 3, self->rows[3]);

	repr= PyObject_Repr(list);
	Py_DECREF(list);
	
	return repr;
}

static PyObject *Matrix_item(MatrixObject *self, int i)
{
	if (i < 0 || i >= 4) {
		PyErr_SetString(PyExc_IndexError, "array index out of range");
		return NULL;
	}
	return py_incr_ret(self->rows[i]);
}

static PySequenceMethods Matrix_SeqMethods = {
	(inquiry) 0,				/*sq_length*/
	(binaryfunc) 0,				/*sq_concat*/
	(intargfunc) 0,				/*sq_repeat*/
	(intargfunc) Matrix_item,	/*sq_item*/
	(intintargfunc) 0,			/*sq_slice*/
	(intobjargproc) 0,			/*sq_ass_item*/
	(intintobjargproc) 0,		/*sq_ass_slice*/
};

PyTypeObject Matrix_Type = {
	PyObject_HEAD_INIT(NULL)
	0,								/*ob_size*/
	"Matrix",						/*tp_name*/
	sizeof(MatrixObject),			/*tp_basicsize*/
	0,								/*tp_itemsize*/
	/* methods */
	(destructor)	Matrix_dealloc,	/*tp_dealloc*/
	(printfunc)		0,				/*tp_print*/
	(getattrfunc)	Matrix_getattr,	/*tp_getattr*/
	(setattrfunc)	Matrix_setattr,	/*tp_setattr*/
	0,								/*tp_compare*/
	(reprfunc)		Matrix_repr,	/*tp_repr*/
	0,								/*tp_as_number*/
	&Matrix_SeqMethods,				/*tp_as_sequence*/
};

void init_py_matrix(void) {
	Matrix_Type.ob_type = &PyType_Type;
}

