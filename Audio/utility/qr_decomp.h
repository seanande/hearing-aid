/* ======================================================= */
/*   START COPY-PASTED QR-DECOMPOSITION CODE                 */
/* ======================================================= */

// Original source: https://rosettacode.org/wiki/QR_decomposition#C

/*
TODO:
  - optimize using CMSIS DSP library (arm_math.h)
*/

#ifndef qr_decomp_h_
#define qr_decomp_h_

#include <stdlib.h>
#include <math.h>

typedef struct {
    int m, n;
    double ** v;
} mat_t, *mat;

inline mat qr_matrix_new(int m, int n)
{
    mat x = (mat) malloc(sizeof(mat_t));
    x->v = (double **) malloc(sizeof(double*) * m);
    x->v[0] = (double *) calloc(sizeof(double), m * n);
    for (int i = 0; i < m; i++)
        x->v[i] = x->v[0] + n * i;
    x->m = m;
    x->n = n;
    return x;
}

inline void qr_matrix_delete(mat m)
{
    free(m->v[0]);
    free(m->v);
    free(m);
}

inline void qr_matrix_transpose(mat m)
{
    for (int i = 0; i < m->m; i++) {
        for (int j = 0; j < i; j++) {
            double t = m->v[i][j];
            m->v[i][j] = m->v[j][i];
            m->v[j][i] = t;
        }
    }
}

inline mat qr_matrix_copy(int n, void *arr, int m)
{
    // workaround for VLA in C++: https://stackoverflow.com/a/25552114
    double (*a)[n] = static_cast<double (*)[n]>(arr);
    mat x = qr_matrix_new(m, n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            x->v[i][j] = a[i][j];
    return x;
}

inline mat qr_matrix_mul(mat x, mat y)
{
    if (x->n != y->m) return 0;
    mat r = qr_matrix_new(x->m, y->n);
    for (int i = 0; i < x->m; i++)
        for (int j = 0; j < y->n; j++)
            for (int k = 0; k < x->n; k++)
                r->v[i][j] += x->v[i][k] * y->v[k][j];
    return r;
}

inline mat qr_matrix_minor(mat x, int d)
{
    mat m = qr_matrix_new(x->m, x->n);
    for (int i = 0; i < d; i++)
        m->v[i][i] = 1;
    for (int i = d; i < x->m; i++)
        for (int j = d; j < x->n; j++)
            m->v[i][j] = x->v[i][j];
    return m;
}

/* c = a + b * s */
inline double *vmadd(double a[], double b[], double s, double c[], int n)
{
    for (int i = 0; i < n; i++)
        c[i] = a[i] + s * b[i];
    return c;
}

/* m = I - v v^T */
inline mat vmul(double v[], int n)
{
    mat x = qr_matrix_new(n, n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            x->v[i][j] = -2 *  v[i] * v[j];
    for (int i = 0; i < n; i++)
        x->v[i][i] += 1;

    return x;
}

/* ||x|| */
inline double vnorm(double x[], int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++) sum += x[i] * x[i];
    return sqrt(sum);
}

/* y = x / d */
inline double* vdiv(double x[], double d, double y[], int n)
{
    for (int i = 0; i < n; i++) y[i] = x[i] / d;
    return y;
}

/* take c-th column of m, put in v */
inline double* mcol(mat m, double *v, int c)
{
    for (int i = 0; i < m->m; i++)
        v[i] = m->v[i][c];
    return v;
}

inline void householder(mat m, mat *R, mat *Q)
{
    mat q[m->m];
    mat z = m, z1;
    for (int k = 0; k < m->n && k < m->m - 1; k++) {
        double e[m->m], x[m->m], a;
        z1 = qr_matrix_minor(z, k);
        if (z != m) qr_matrix_delete(z);
        z = z1;

        mcol(z, x, k);
        a = vnorm(x, m->m);
        if (m->v[k][k] > 0) a = -a;

        for (int i = 0; i < m->m; i++)
            e[i] = (i == k) ? 1 : 0;

        vmadd(x, e, a, e, m->m);
        vdiv(e, vnorm(e, m->m), e, m->m);
        q[k] = vmul(e, m->m);
        z1 = qr_matrix_mul(q[k], z);
        if (z != m) qr_matrix_delete(z);
        z = z1;
    }
    qr_matrix_delete(z);
    *Q = q[0];
    *R = qr_matrix_mul(q[0], m);
    for (int i = 1; i < m->n && i < m->m - 1; i++) {
        z1 = qr_matrix_mul(q[i], *Q);
        if (i > 1) qr_matrix_delete(*Q);
        *Q = z1;
        qr_matrix_delete(q[i]);
    }
    qr_matrix_delete(q[0]);
    z = qr_matrix_mul(*Q, m);
    qr_matrix_delete(*R);
    *R = z;
    qr_matrix_transpose(*Q);
}

#endif
