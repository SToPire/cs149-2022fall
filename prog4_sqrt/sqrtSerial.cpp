#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


void sqrtSerial(int N,
                float initialGuess,
                float values[],
                float output[])
{

    static const float kThreshold = 0.00001f;

    for (int i=0; i<N; i++) {

        float x = values[i];
        float guess = initialGuess;

        float error = fabs(guess * guess * x - 1.f);

        while (error > kThreshold) {
            guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
            error = fabs(guess * guess * x - 1.f);
        }

        output[i] = x * guess;
    }
}

__m256 _avx_fabs(__m256 input) {
    /*
     *  if (input[i] > 0)
     *      res[i] = input[i];
     *  else
     *      res[i] = -input[i];
     */
    __m256 cmpRes = _mm256_cmp_ps(input, _mm256_setzero_ps(), _CMP_GT_OQ);
    __m256 negInput = _mm256_sub_ps(_mm256_setzero_ps(), input);
    return _mm256_blendv_ps(negInput, input, cmpRes);
}

void sqrtAVX2(int N, float initialGuess, float values[], float output[]) {
    static const float kThreshold = 0.00001f;

    // 256 / sizeof(float) == 8
    const int VECTOR_WIDTH = 8;

    __m256 x, guess, error;
    __m256 cmpRes;

    /* Assume VECTOR_WIDTH divides N */
    for (int i = 0; i < N; i += VECTOR_WIDTH) {
        guess = _mm256_set1_ps(initialGuess);
        // x = values[i];
        x = _mm256_loadu_ps(values + i);

        // error = fabs(guess * guess * x - 1.0f);
        error = _mm256_mul_ps(guess, guess);
        error = _mm256_mul_ps(error, x);
        error = _mm256_sub_ps(error, _mm256_set1_ps(1.0f));
        error = _avx_fabs(error);

        // while (error > kThreshold)
        cmpRes = _mm256_cmp_ps(error, _mm256_set1_ps(kThreshold), _CMP_GT_OQ);
        while (_mm256_testz_ps(cmpRes, cmpRes) == 0) {
            // use temp variables, with cmpRes as mask to protect irrelavant components
            __m256 curGuess = _mm256_and_ps(guess, cmpRes);
            __m256 curError = _mm256_and_ps(error, cmpRes);

            // guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
            __m256 tmp1 = _mm256_mul_ps(curGuess, _mm256_set1_ps(3.0f));
            __m256 tmp2 = _mm256_mul_ps(_mm256_mul_ps(_mm256_mul_ps(x, curGuess), curGuess), curGuess);
            __m256 tmp3 = _mm256_sub_ps(tmp1, tmp2);
            curGuess = _mm256_mul_ps(tmp3, _mm256_set1_ps(0.5f));

            // error = fabs(guess * guess * x - 1.f);
            curError = _mm256_mul_ps(curGuess, curGuess);
            curError = _mm256_mul_ps(curError, x);
            curError = _mm256_sub_ps(curError, _mm256_set1_ps(1.0f));
            curError = _avx_fabs(curError);

            // write back, using cmpRes as mask
            guess = _mm256_blendv_ps(guess, curGuess, cmpRes);
            error = _mm256_blendv_ps(error, curError, cmpRes);

            // re-calculate cmpRes
            cmpRes = _mm256_cmp_ps(error, _mm256_set1_ps(kThreshold), _CMP_GT_OQ);
        }

        // ourput[i] = x * guess;
        _mm256_storeu_ps(output + i, _mm256_mul_ps(x, guess));
    }
}

void sqrtAVXNative(int N, float initialGuess, float values[], float output[]) {
    const int VECTOR_WIDTH = 8;
    for (int i = 0; i < N; i += VECTOR_WIDTH) {
        _mm256_storeu_ps(output + i, _mm256_sqrt_ps(_mm256_loadu_ps(values + i)));
    }
}