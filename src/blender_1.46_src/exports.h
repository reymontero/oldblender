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




/*
 * exports.h
 * 
 * jan 95
 * 
 * 
 */

	/* anim.c */
extern int where_on_path(Object *ob, float ctime, float *vec, float *dir);

	/* arithb.c */
extern void CalcCent3f(float *cent, float *v1, float *v2, float *v3);
extern void CalcCent4f(float *cent, float *v1, float *v2, float *v3, float *v4);
extern void Crossf(float *c, float *a, float *b);
extern void EulToMat3(float *eul, float mat[][3]);
extern void QuatToEul(float *quat, float *eul);
extern void Mat3Inv(float m1[][3], float m2[][3]);
extern int Mat4Invert(float inverse[][4], float mat[][4]);
extern void Mat3CpyMat4(float m1[][3], float m2[][4]);
extern void Mat3ToEul(float tmat[][3], float *eul);
/* extern void Mat3MulMat3(float *m1, float *m3, float *m2); */
/* extern void Mat3MulVecfl(float mat[][3], float *vec); */
extern void Mat4MulVecfl(float mat[][4], float *vec);
extern void Mat3MulFloat(float *m, float f);
extern void Mat4MulFloat(float *m, float f);
extern void Mat4MulFloat3(float *m, float f);
extern int FloatCompare(float *v1, float *v2, float limit);
extern float Normalise(float *n);
extern float CalcNormFloat(float *v1,float *v2,float *v3,float *n);
extern float CalcNormFloat4(float *v1,float *v2,float *v3,float *v4,float *n);
extern float VecLenf(float *v1, float *v2);
extern void VecMulf(float *v1, float f);
extern float Sqrt3f(float f);
extern double Sqrt3d(double d);
extern void euler_rot(float *beul, float ang, char axis);
extern    float safacos(float fac);
extern float Inpf(float *v1, float *v2);
extern void VecSubf(register float *v, register float *v1, register float *v2);

extern float DistVL2Dfl(float *v1,float *v2,float *v3);
extern float PdistVL2Dfl(float *v1,float *v2,float *v3);
extern float AreaF2Dfl(float *v1, float *v2, float *v3);
extern float AreaQ3Dfl(float *v1, float *v2, float *v3, float *v4);
extern float AreaT3Dfl(float *v1, float *v2, float *v3);
extern float AreaPoly3Dfl(int nr, float *verts, float *normal);
extern void VecRotToMat3(float *vec, float phi, float mat[][3]);
extern float Spec(float inp, int hard);
extern float *vectoquat(float *vec, short axis, short upflag);

extern void i_lookat(float vx, float vy, float vz, float px, float py, float pz, float twist, float mat[][4]);
extern void i_window(float left, float right, float bottom, float top, float near, float far, float mat[][4]);

extern void hsv_to_rgb(float h, float s, float v, float *r, float *g, float *b);
extern void rgb_to_hsv(float r, float g, float b, float *lh, float *ls, float *lv);
extern uint hsv_to_cpack(float h, float s, float v);
extern uint rgb_to_cpack(float r, float g, float b);
extern void cpack_to_rgb(uint col, float *r, float *g, float *b);

	/* blender.c */
extern struct Global G;
extern float matone[4][4];
extern char versionstr[];
extern void *dupallocN(void *mem);
extern int alloc_len(void *mem);

	/* buttons.c */
extern short button(short *var, short min, short max, char *str);
extern void add_numbut(int nr, int type, char *str, float min, float max, void *poin, char *tip);

	/* curve.c */
extern Curve *add_curve();
extern Curve *copy_curve(Curve *cu);
extern Nurb *duplicateNurb(Nurb *nu);
extern int count_curveverts(ListBase *nurb);
extern void autocalchandlesNurb(Nurb *nu, int flag);
extern void autocalchandlesNurb_all(int flag);
extern void calchandleNurb(BezTriple *bezt,BezTriple *prev, BezTriple *next, int mode);
extern void calchandlesNurb(Nurb *nu);
extern void freeNurblist(ListBase *lb);
extern void duplicateNurblist(ListBase *lb1, ListBase *lb2);
extern void freeNurb(Nurb *nu);
extern void free_curve(Curve *cu);
extern void maakbez(float q0, float q1, float q2, float q3, float *p, int it);
extern void makeBevelList(Object *ob);
extern void makeNurbcurve(Nurb *nu, float *data, int dim);
extern void makeNurbfaces(Nurb *nu, float *data) ;
extern void make_local_curve(Curve *cu);
extern void make_orco_surf(Curve *cu);
extern void makebevelcurve(Object *ob, ListBase *disp);
extern void makeknots(Nurb *nu, short uv, short type);
extern void minmaxNurb(Nurb *nu, float *min, float *max);
extern void sethandlesNurb(short code);
extern void switch_endian_knots(Nurb *nu);
extern void test2DNurb(Nurb *nu);
extern void test_curve_type(Object *ob);
extern void testhandlesNurb(Nurb *nu);
extern void tex_space_curve(Curve *cu);
extern void unlink_curve(Curve *cu);


	/* displist.c */
extern void filldisplist(ListBase *dispbase, ListBase *to);
extern DispList *find_displist_create(ListBase *lb, int type);
extern DispList *test_displist(ListBase *lb, int type);
extern void addnormalsDispList(Object *ob, ListBase *lb);
extern void count_displist(ListBase *lb, int *totvert, int *totface);
extern void curve_to_filledpoly(Curve *cu, ListBase *dispbase);
extern void fastshade(float *co, float *nor, float *orco, Material *ma, char *col1, char *col2, char *vertcol);
extern void freedisplist(ListBase *lb);
extern void makeDispList(Object *ob);
extern void set_displist_onlyzero(int val);
extern void shadeDispList(Object *ob);
	/* drawimage.c */
extern void rectwrite_part(int winxmin, int winymin, int winxmax, int winymax, int x1, int y1, int xim, int yim, float zoom, ulong *rect);

	/* drawipo.c */
extern void view2dzoom();

	/* drawobject.c */
extern ulong rectpurple[5][5];
extern ulong rectyellow[5][5];
extern void tekenrect_col(short size, short sx, short sy, ulong col);
extern void drawaxes(float size);
extern void drawcircball(float *cent, float rad, float tmat[][4]);

	/* drawview.c */
extern double speed_to_swaptime(int speed);

	/* editcurve.c */
extern ListBase editNurb;
extern void addprimitiveCurve(int stype);
extern void addprimitiveNurb(int type);

	/* editipo.c */
extern Ipo *get_ipo_to_edit(ID **from);
extern void set_speed_editipo(float speed);

	/* editkey.c */
extern Key *give_current_key(Object *ob);

	/* editlattice.c */
extern Lattice *editLatt;
extern Lattice *add_lattice();

	/* editmball.c */
extern ListBase editelems;
extern void add_primitiveMball();

	/* editmesh.c ook in edit.h */
extern void add_primitiveMesh(int type);
extern void fasterdraw();
extern void slowerdraw();
extern void make_sticky();
extern void make_vertexcol();
extern void vertexsmooth();
extern void vertexnoise();
extern void subdivideflag(int flag, float rad, int beauty);

	/* editobject.c */
extern void docentre();
extern void docentre_new();
extern void add_object_draw(int type);
extern void apply_keyb_grid(float *val, float fac1, float fac2, float fac3, int invert);
extern Object *find_camera();
	
	/* editview.c */
extern void draw_sel_circle(short *mval, short *mvalo, float rad, float rado, int selecting);

	/* effect.c in effect.h */
	
	/* envmap.c */
extern EnvMap *add_envmap(void);
extern EnvMap *copy_envmap(EnvMap *ema);
extern void free_envmap(EnvMap *ema);


	/* exotic.c */
extern void read_exotic(char *name);
extern void write_videoscape(char *str);
extern void dxf_read(char *str);
extern void write_videoscape_fs(void);
extern void write_vrml_fs(void);
extern void write_dxf_fs(void);

	/* filesel.c */
extern void addfilename_to_fsmenu(char *name);
extern void filesel_statistics(SpaceFile *sfile, int *totfile, int *selfile, float *totlen, float *sellen);
extern void sort_filelist(SpaceFile *sfile);
extern void split_dirfile(char *string, char *dir, char *file);
extern void read_dir(SpaceFile *sfile);
extern void freefilelist(SpaceFile *sfile);
extern void parent(SpaceFile *sfile);
extern void swapselect_file(SpaceFile *sfile);
extern void activate_fileselect(int type, char *title, char *file, void (*func)(char *));
extern void activate_imageselect(int type, char *title, char *file, void (*func)(char *));
extern void activate_databrowse(ID *id, int idcode, int fromcode, int retval, void (*func)(ushort));

extern void free_filesel_spec(char *dir);
extern void winqreadfilespace(ushort event, short val);
extern int groupname_to_code(char *group);
extern char *code_to_groupname(int code);
extern int is_a_library(SpaceFile *sfile, char *dir, char *group);
extern void make_file_string (char *string,  char *dir,  char *file);

	/* fileops.c */
extern int fop_delete(char *file, int dir, int recursive);
extern int fop_touch(char *file);
extern int fop_move(char *file, char *to);
extern int fop_copy(char *file, char *to);
extern int fop_link(char *file, char *to);
extern int fop_backup(char *file, char *from, char *to);
extern int fop_exists(char *file);
extern void fop_recurdir(char *dirname);
extern int fop_rename(char *from, char *to);
	
	/* font.c */
extern VFont *load_vfont(char *name);

	/* image.c */
extern void free_image(Image *ima);
extern void makepicstring(char *string, int frame);
extern void write_ibuf(ImBuf *ibuf, char *name);
extern int calcimanr(int cfra, Tex *tex);
extern void ima_ibuf_is_nul(Tex *tex);
extern int imagewrap(Tex *tex, float *texvec);
extern int imagewraposa(Tex *tex, float *texvec, float *dxt, float *dyt);
extern Image *add_image(char *str);
extern struct anim *openanim(char * name, int flags);

	/* imageprocess.c */
extern void scalefastrect(uint *recto, uint *rectn, int oldx, int oldy, int newx, int newy);
extern void addalphaAdd(char *doel, char *bron);
/* extern void addalphaAddfac(char *doel, char *bron, char addfac); */
/* extern void addalphaOver(char *doel, char *bron); */
extern void addalphaUnder(char *doel, char *bron);
extern void addalphaUnderGamma(char *doel, char *bron);
extern void keyalpha (char *doel);
	

	/* ipo.c */
extern float frame_to_float(int cfra);
extern void calc_ipo(Ipo *ipo, float ctime);
extern Ipo *add_ipo(char *name, int idcode);
extern float read_ipo_poin(void *poin, int type);
extern void write_ipo_poin(void *poin, int type, float val);
extern void *get_ipo_poin(ID *id, IpoCurve *icu, int *type);
extern Ipo *copy_ipo(Ipo *ipo);
extern void correct_bezpart(float *v1, float *v2, float *v3, float *v4);
extern int calc_ipo_spec(Ipo *ipo, int adrcode, float *ctime);

	/* isect.c */
extern void intersect_mesh();

	/* initrender.c */
extern float safacos(float fac);

	/* key.c */
extern Key *add_key(ID *id);
extern Key *copy_key(Key *key);
extern void set_four_ipo(float d, float *data, int type);
extern void set_afgeleide_four_ipo(float d, float *data, int type);

	/* library.c */
extern void *alloc_libblock(ListBase *lb, short type, char *name);
extern ID *alloc_libblock_notest(short type);
extern void *copy_libblock(void *rt);
extern void test_idbutton(char *name);
extern ListBase *wich_libbase(Main *main, short type);
extern Main *find_main(char *dir);
extern int has_id_number(ID *id);
extern ID *find_id(char *type, char *name);

	/* material.c */
extern Material *add_material(char *name);
extern Material *copy_material(Material *ma);
extern Material *give_current_material(Object *ob, int act);
extern ID *material_from(Object *ob, int act);
extern Material ***give_matarar(Object *ob);
extern short *give_totcolp(Object *ob);

	/* mball.c */
extern MetaBall *add_mball();
extern MetaBall *copy_mball(MetaBall *mb);
extern Object *find_basis_mball(Object *ob);
extern void tex_space_mball(Object *ob);

	/* mesh.c */
extern Mesh *add_mesh();
extern Mesh *copy_mesh(Mesh *me);
extern Mesh *get_mesh(Object *ob);
extern Mesh *get_other_mesh(Object *ob);

	/* noise.c */
extern float hnoise(float noisesize, float x, float y, float z);
extern float hnoisep(float noisesize, float x, float y, float z);
extern float turbulence(float noisesize, float x, float y, float z, int nr);
extern float turbulence1(float noisesize, float x, float y, float z, int nr);

	/* object.c */
extern Object workob;
extern Object *add_object(int type);
extern Lamp *copy_lamp(Lamp *la);
extern Lattice *copy_lattice(Lattice *lt);
extern Camera *copy_camera(Camera *cam);
extern Object *copy_object(Object *ob);
extern float system_time(Object *ob, Object *par, float cfra, float ofs);
extern BoundBox *unit_boundbox();
extern void where_is_object_time(Object *ob, float ctime);

	/* rct.c */
extern void init_rctf(rctf *rect, float xmin, float xmax, float ymin, float ymax);
extern int in_rctf(rctf *rect, float x, float y);
extern isect_rctf(rctf *src1, rctf *src2, rctf *dest);

	/* readfile.c */
extern int convertstringcode(char *str);
extern void makestringcode(char *str);

	/* scene.c */
extern Scene *add_scene(char *name);
extern Scene *copy_scene(Scene *sce, int level);

	/* scanfill.c */
extern void *new_mem_element(int size);

	/* sector.c */
extern void view_to_piramat(float mat[][4], float lens, float far);
	
	/* texture.c */
extern Tex *add_texture(char *name);
extern MTex *add_mtex();
extern Tex *copy_texture(Tex *tex);
extern PluginTex *add_plugin_tex(char *str);
extern ColorBand *add_colorband();

	/* usiblender.c */
extern UserDef U;

	/* view.c */
extern float *give_cursor();

	/* world.c */
extern World *add_world(char *name);
extern World *copy_world(World *wrld);

	/* writeblendpsx.c */
extern int le_coordint(float ftemp);
extern short le_coordshort(float ftemp);
extern short le_float_dangshort(float ftemp);
extern short le_floatangshort(float ftemp);
extern int le_dyna_int(float ftemp);
extern short le_dyna_short(float ftemp);
extern short le_floatshort(float ftemp);

	/* zbuf.c */
extern int testclip(float *v);



