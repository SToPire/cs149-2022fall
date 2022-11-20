#include <stdio.h>
#include <algorithm>
#include <pthread.h>
#include <math.h>

#include "CycleTimer.h"
#include "sqrt_ispc.h"

using namespace ispc;

extern void sqrtSerial(int N, float startGuess, float* values, float* output);
extern void sqrtAVX2(int N, float initialGuess, float* values, float* output);
extern void sqrtAVXNative(int N, float initialGuess, float values[], float output[]);

static void verifyResult(int N, float* result, float* gold) {
    for (int i=0; i<N; i++) {
        if (fabs(result[i] - gold[i]) > 1e-4) {
            printf("Error: [%d] Got %f expected %f\n", i, result[i], gold[i]);
        }
    }
}

int main() {

    const unsigned int N = 20 * 1000 * 1000;
    const float initialGuess = 1.0f;

    float* values = new float[N];
    float* output = new float[N];
    float* gold = new float[N];

    for (unsigned int i=0; i<N; i++)
    {
        // TODO: CS149 students.  Attempt to change the values in the
        // array here to meet the instructions in the handout: we want
        // to you generate best and worse-case speedups
        
        // starter code populates array with random input values
        values[i] = .001f + 2.998f * static_cast<float>(rand()) / RAND_MAX;

        // best case: all values are same, with a high interation count.
        // values[i] = 2.99999;

        // worst case 1: serial exec is already fast
        // values[i] = 1.0f;

        // worst case 2: a slow 2.99999 become the bottle neck
        // values[i] = i%8 ? 1 : 2.99999;
    }

    // generate a gold version to check results
    for (unsigned int i=0; i<N; i++)
        gold[i] = sqrt(values[i]);

    //
    // And run the serial implementation 3 times, again reporting the
    // minimum time.
    //
    double minSerial = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrtSerial(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minSerial = std::min(minSerial, endTime - startTime);
    }

    printf("[sqrt serial]:\t\t[%.3f] ms\n", minSerial * 1000);

    verifyResult(N, output, gold);

    //
    // Compute the image using the ispc implementation; report the minimum
    // time of three runs.
    //
    double minISPC = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrt_ispc(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minISPC = std::min(minISPC, endTime - startTime);
    }

    printf("[sqrt ispc]:\t\t[%.3f] ms\n", minISPC * 1000);

    verifyResult(N, output, gold);

    // Clear out the buffer
    for (unsigned int i = 0; i < N; ++i)
        output[i] = 0;

    //
    // Tasking version of the ISPC code
    //
    double minTaskISPC = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrt_ispc_withtasks(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minTaskISPC = std::min(minTaskISPC, endTime - startTime);
    }

    printf("[sqrt task ispc]:\t[%.3f] ms\n", minTaskISPC * 1000);

    verifyResult(N, output, gold);

    // Clear out the buffer
    for (unsigned int i = 0; i < N; ++i)
        output[i] = 0;

    double minAVX = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrtAVX2(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minAVX = std::min(minAVX, endTime - startTime);
    }

    printf("[sqrt AVX]:\t[%.3f] ms\n", minAVX * 1000);

    verifyResult(N, output, gold);

    // Clear out the buffer
    for (unsigned int i = 0; i < N; ++i)
        output[i] = 0;

    double minAVXNative = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        sqrtAVXNative(N, initialGuess, values, output);
        double endTime = CycleTimer::currentSeconds();
        minAVXNative = std::min(minAVXNative, endTime - startTime);
    }

    printf("[sqrt AVX native]:\t[%.3f] ms\n", minAVXNative * 1000);

    verifyResult(N, output, gold);

    printf("\t\t\t\t(%.2fx speedup from ISPC)\n", minSerial/minISPC);
    printf("\t\t\t\t(%.2fx speedup from task ISPC)\n", minSerial/minTaskISPC);
    printf("\t\t\t\t(%.2fx speedup from AVX)\n", minSerial/minAVX);
    printf("\t\t\t\t(%.2fx speedup from AVX native)\n", minSerial/minAVXNative);

    delete [] values;
    delete [] output;
    delete [] gold;

    return 0;
}
