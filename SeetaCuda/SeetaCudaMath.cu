


#include "SeetaCudaMath.h"
#include <cublas_v2.h> //cuda×Ô´ø¿âº¯Êý  
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

SeetaCudaMath::SeetaCudaMath()
{
}

__global__ void matrix_procuct_kernel(const float* A, const float* B, float* C,int n, int m, int
	 k)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < m * n)
	{

		int iM = idx / n;
		int iN = idx%n;
		if (iM < m)
		{
			float sum = 0;

			for (int i = 0; i < k; i++)
			{
				sum += B[iM * k + i] * A[iN*k
					+ i];
			}

			C[iM*n + iN] = sum;
		}
	}


}
//// C = B*A^T
//void SeetaCudaMath::matrix_procuct(int cudaDevice, const float* A, const float* B, float* C, const int n,
//	const int m, const int k)
//{
//	cudaSetDevice(cudaDevice);
//
//	float *dev_a = 0;
//	float *dev_b = 0;
//	float *dev_c = 0;
//
//
//	cudaMalloc((void**)&dev_a,  n * k * sizeof(float));
//	cudaMemcpy(dev_a, A, n * k * sizeof(float), cudaMemcpyHostToDevice);
//	cudaMalloc((void**)&dev_b,  m * k * sizeof(float));
//	cudaMemcpy(dev_b, B, m * k * sizeof(float), cudaMemcpyHostToDevice);
//	cudaMalloc((void**)&dev_c,  m * n * sizeof(float));
//
//	dim3 grid(1024, 1, 1), dimBlock(n*m / 1024 + 1);
//	matrix_procuct_kernel << <grid, dimBlock >> >(dev_a, dev_b, dev_c, n, m, k);
//	cudaThreadSynchronize();
//	cudaMemcpy(C, dev_c, m * n * sizeof(float), cudaMemcpyDeviceToHost);
//	cudaFree(dev_a);
//	cudaFree(dev_b);
//	cudaFree(dev_c);
//
//}

float simd_dot(const float* x, const float* y, const long& len) {
	float inner_prod = 0.0f;
	__m128 X, Y; // 128-bit values
	__m128 acc = _mm_setzero_ps(); // set to (0, 0, 0, 0)
	float temp[4];

	long i;
	for (i = 0; i + 4 < len; i += 4) {
		X = _mm_loadu_ps(x + i); // load chunk of 4 floats
		Y = _mm_loadu_ps(y + i);
		acc = _mm_add_ps(acc, _mm_mul_ps(X, Y));
	}
	_mm_storeu_ps(&temp[0], acc); // store acc into an array
	inner_prod = temp[0] + temp[1] + temp[2] + temp[3];

	// add the remaining values
	for (; i < len; ++i) {
		inner_prod += x[i] * y[i];
	}
	return inner_prod;
}
void SeetaCudaMath::matrix_procuct_cpu(const float* A, const float* B, float* C, const int n,
	const int m, const int k) {

	const float* x = B;
	for (int i = 0, idx = 0; i < m; ++i) {
		const float* y = A;
		for (int j = 0; j < n; ++j, ++idx) {
			C[idx] = simd_dot(x, y, k);
			y += k;
		}
		x += k;
	}
}

void SeetaCudaMath::resizeImgCpu(unsigned char* src_data, int src_width, int src_height, unsigned char* dest_data, int dest_width, int dest_height)
{
	double lf_x_scl = static_cast<double>(src_width) / dest_width;
	double lf_y_Scl = static_cast<double>(src_height) / dest_height;
	  for (int y = 0; y < dest_height; y++) {
      for (int x = 0; x < dest_width; x++) {
        double lf_x_s = lf_x_scl * x;
        double lf_y_s = lf_y_Scl * y;

        int n_x_s = static_cast<int>(lf_x_s);
        n_x_s = (n_x_s <= (src_width - 2) ? n_x_s : (src_width - 2));
        int n_y_s = static_cast<int>(lf_y_s);
        n_y_s = (n_y_s <= (src_height - 2) ? n_y_s : (src_height - 2));

        double lf_weight_x = lf_x_s - n_x_s;
        double lf_weight_y = lf_y_s - n_y_s;

        double dest_val = (1 - lf_weight_y) * ((1 - lf_weight_x) *
          src_data[n_y_s * src_width + n_x_s] +
          lf_weight_x * src_data[n_y_s * src_width + n_x_s + 1]) +
          lf_weight_y * ((1 - lf_weight_x) * src_data[(n_y_s + 1) * src_width + n_x_s] +
          lf_weight_x * src_data[(n_y_s + 1) * src_width + n_x_s + 1]);

		dest_data[y * dest_width + x] = static_cast<unsigned char>(dest_val);
      }
    }
}

__global__ void resize_img_kernel(unsigned char* src_data, int src_width, int src_height, unsigned char* dest_data, int dest_width, int dest_height, float lf_x_scl, float lf_y_Scl)
{
	int y = blockIdx.x;
	int x = threadIdx.x;

	float lf_x_s = lf_x_scl * x;
	float lf_y_s = lf_y_Scl * y;

	int n_x_s = static_cast<int>(lf_x_s);
	n_x_s = (n_x_s <= (src_width - 2) ? n_x_s : (src_width - 2));
	int n_y_s = static_cast<int>(lf_y_s);
	n_y_s = (n_y_s <= (src_height - 2) ? n_y_s : (src_height - 2));

	float lf_weight_x = lf_x_s - n_x_s;
	float lf_weight_y = lf_y_s - n_y_s;

	float dest_val = (1 - lf_weight_y) * ((1 - lf_weight_x) *
		src_data[n_y_s * src_width + n_x_s] +
		lf_weight_x * src_data[n_y_s * src_width + n_x_s + 1]) +
		lf_weight_y * ((1 - lf_weight_x) * src_data[(n_y_s + 1) * src_width + n_x_s] +
		lf_weight_x * src_data[(n_y_s + 1) * src_width + n_x_s + 1]);

	dest_data[y * dest_width + x] = static_cast<unsigned char>(dest_val);
}

void SeetaCudaMath::resizeImgGpu(int cudaDevice, unsigned char* src_data, int src_width, int src_height, unsigned char* dest_data, int dest_width, int dest_height)
{
	cudaSetDevice(cudaDevice);

	double lf_x_scl = static_cast<double>(src_width) / dest_width;
	double lf_y_scl = static_cast<double>(src_height) / dest_height;


	unsigned char *dev_src = 0;
	unsigned char *dev_dest = 0;

	cudaMalloc((void**)&dev_src, src_width * src_height * sizeof(unsigned char));
	cudaMemcpy(dev_src, src_data, src_width * src_height * sizeof(unsigned char), cudaMemcpyHostToDevice);
	cudaMalloc((void**)&dev_dest, dest_width * dest_height * sizeof(unsigned char));
	dim3 grid(dest_height, 1, 1), dimBlock(dest_width);
	resize_img_kernel << <grid, dimBlock >> >(dev_src, src_width, src_height, dev_dest, dest_width, dest_height, lf_x_scl, lf_y_scl);
	cudaThreadSynchronize();
	cudaMemcpy(dest_data, dev_dest, dest_width * dest_height * sizeof(unsigned char), cudaMemcpyDeviceToHost);
	cudaFree(dev_src);
	cudaFree(dev_dest);

}
//×ó³Ë
void SeetaCudaMath::AnkMutBkm(int cudaDevice, const float* A, const float* B, float* C, const int n,
	const int m, const int k)
{
	cudaSetDevice(cudaDevice);

	float *dev_a = 0;
	float *dev_b = 0;
	float *dev_c = 0;


	cudaMalloc((void**)&dev_a, n * k * sizeof(float));
	cudaMemcpy(dev_a, A, n * k * sizeof(float), cudaMemcpyHostToDevice);
	cudaMalloc((void**)&dev_b, m * k * sizeof(float));
	cudaMemcpy(dev_b, B, m * k * sizeof(float), cudaMemcpyHostToDevice);
	cudaMalloc((void**)&dev_c, m * n * sizeof(float));
	float alpha = 1.0;
	float beta = 0.0;
	cublasHandle_t handle;
	cublasCreate(&handle);
	cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N, m, n, k, &alpha, dev_b, m, dev_a, k, &beta, dev_c, m);

	cudaThreadSynchronize();
	cudaMemcpy(C, dev_c, m * n * sizeof(float), cudaMemcpyDeviceToHost);
	cudaFree(dev_a);
	cudaFree(dev_b);
	cudaFree(dev_c);
}

void SeetaCudaMath::AnkMutTBmk(int cudaDevice, const float* A, const float* B, float* C, const int n,
	const int m, const int k)
{
	cudaSetDevice(cudaDevice);

	float *dev_a = 0;
	float *dev_b = 0;
	float *dev_c = 0;


	cudaMalloc((void**)&dev_a, n * k * sizeof(float));
	cudaMemcpy(dev_a, A, n * k * sizeof(float), cudaMemcpyHostToDevice);
	cudaMalloc((void**)&dev_b, m * k * sizeof(float));
	cudaMemcpy(dev_b, B, m * k * sizeof(float), cudaMemcpyHostToDevice);
	cudaMalloc((void**)&dev_c, m * n * sizeof(float));
	float alpha = 1.0;
	float beta = 0.0;
	cublasHandle_t handle;
	cublasCreate(&handle);
	cublasSgemm(handle, CUBLAS_OP_T, CUBLAS_OP_N, m, n, k, &alpha, dev_b, k, dev_a, k, &beta, dev_c, m);
	cudaThreadSynchronize();
	cudaMemcpy(C, dev_c, m * n * sizeof(float), cudaMemcpyDeviceToHost);
	cudaFree(dev_a);
	cudaFree(dev_b);
	cudaFree(dev_c);
}
//C = B*A^T
void SeetaCudaMath::matrix_procuct_gpu(int cudaDevice, const float* A, const float* B, float* C, const int n,
	const int m, const int k)
{

	AnkMutTBmk(cudaDevice,B, A, C, m, n, k);
	/*cudaSetDevice(cudaDevice);

	float *dev_a = 0;
	float *dev_b = 0;
	float *dev_c = 0;


	cudaMalloc((void**)&dev_a, n * k * sizeof(float));
	cudaMemcpy(dev_a, A, n * k * sizeof(float), cudaMemcpyHostToDevice);
	cudaMalloc((void**)&dev_b, m * k * sizeof(float));
	cudaMemcpy(dev_b, B, m * k * sizeof(float), cudaMemcpyHostToDevice);
	cudaMalloc((void**)&dev_c, m * n * sizeof(float));
	float alpha = 1.0;
	float beta = 0.0;
	cublasHandle_t handle;
	cublasCreate(&handle);
	cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_T, m, n, k, &alpha, dev_a, n, dev_b, m, &beta, dev_c, n);

	cudaThreadSynchronize();
	cudaMemcpy(C, dev_c, m * n * sizeof(float), cudaMemcpyDeviceToHost);
	cudaFree(dev_a);
	cudaFree(dev_b);
	cudaFree(dev_c);*/
}
