#include <immintrin.h>

// gemm -- general double precision dense matrix-matrix multiplication.
//
// implement: C = alpha * A x B + beta * C, for matrices A, B, C
// Matrix C is M x N  (M rows, N columns)
// Matrix A is M x K
// Matrix B is K x N
//
// Your implementation should make no assumptions about the values contained in any input parameters.

/* caller must ensure: m == n == k && n is power of 2 */
void gemm(int m, int n, int k, double *A, double *B, double *C, double alpha, double beta){
  /* 256 / sizeof(double) == 4 */
  const int step = 4;

  __m256d vec1, vec2, res;
  __m256d vec_alpha = _mm256_set1_pd(alpha);
  __m256d vec_beta = _mm256_set1_pd(beta);

  // C = beta * C
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j += step) {
      vec1 = _mm256_loadu_pd(&C[i * n + j]);
      vec1 = _mm256_mul_pd(vec1, vec_beta);
      _mm256_storeu_pd(&C[i * n + j], vec1);
    }
  }

  // C += alpha * A x B
  for (int i = 0; i < n; i++) {
    for (int k = 0; k < n; k++) {
      vec1 = _mm256_set1_pd(A[i * n + k]);
      for (int j = 0; j < n; j += step) {
        vec2 = _mm256_loadu_pd(&B[k * n + j]);
        res =
            _mm256_add_pd(_mm256_loadu_pd(&C[i * n + j]),
                          _mm256_mul_pd(vec_alpha, _mm256_mul_pd(vec1, vec2)));
        _mm256_storeu_pd(&C[i * n + j], res);
      }
    }
  }
}

