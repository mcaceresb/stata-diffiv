#ifndef DIFFTOOLS_H
#define DIFFTOOLS_H    1

#define BUF_MAX 4096

void sf_printf (const char *fmt, ...);
void sf_running_timer (clock_t *timer, const char *msg);

ST_retcode sf_get_vector_bool (char *st_matrix, GT_bool *v);
ST_retcode sf_scalar_size (char *st_scalar, GT_size *sval);
ST_retcode sf_oom_error (char *step_desc, char *obj_desc);

GT_size * sf_panelsetup(GT_bool *group, GT_size N, GT_size *G);

#endif /* difftools.h */
