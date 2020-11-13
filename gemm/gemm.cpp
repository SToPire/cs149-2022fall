void gemm(int m, int n, int k, double *A, double *B, double *C, double alpha, double beta){
	// REPLACE THIS WITH YOUR IMPLEMENTATION
	int i, j, kk;
	for(i=0;i<m;i++){
		for(j=0;j<n;j++){
			double inner_prod = 0;
			for(kk=0;kk<k;kk++){
				inner_prod += A[i*k+kk] * B[kk*n+j];
			}
			C[i*n+j] = alpha * inner_prod + beta * C[i*n+j];
		}
	}
	// END OF NAIVE IMPLEMENTATION
}

