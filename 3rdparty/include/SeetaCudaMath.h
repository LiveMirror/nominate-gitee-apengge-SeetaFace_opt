#ifndef __SEETA_CUDA_MATH_H__
#define __SEETA_CUDA_MATH_H__

#define DLL_IMPORT_API __declspec(dllimport)


class DLL_IMPORT_API SeetaCudaMath
{
public:
	SeetaCudaMath();
	//B*A(T)
	static void matrix_procuct_cpu(const float* A, const float* B, float* C, const int n,
		const int m, const int k);
	////B*A(T)
	//static void matrix_procuct2(int cudaDevice, const float* A, const float* B, float* C, const int n,
	//	const int m, const int k);

	//B*A(T)
	static void matrix_procuct_gpu(int cudaDevice, const float* A, const float* B, float* C, const int n,
		const int m, const int k);
	//A*B
	static void AnkMutBkm(int cudaDevice, const float* A, const float* B, float* C, const int n,
		const int m, const int k);
	//A*T(B)
	static void AnkMutTBmk(int cudaDevice, const float* A, const float* B, float* C, const int n,
		const int m, const int k);

	static void resizeImgCpu(unsigned char* src_data, int src_width, int src_height, unsigned char* dest_data, int dest_width, int dest_height);

	static void resizeImgGpu(int cudaDevice, unsigned char* src_data, int src_width, int src_height, unsigned char* dest_data, int dest_width, int dest_height);
	//int cudaDevice;
};


#endif