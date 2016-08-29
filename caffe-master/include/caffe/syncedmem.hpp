#ifndef CAFFE_SYNCEDMEM_HPP_
#define CAFFE_SYNCEDMEM_HPP_

#include <cstdlib>

#include "caffe/common.hpp"

namespace caffe {

// If CUDA is available and in GPU mode, host memory will be allocated pinned,
// using cudaMallocHost. It avoids dynamic pinning for transfers (DMA).
// The improvement in performance seems negligible in the single GPU case,
// but might be more significant for parallel training. Most importantly,
// it improved stability for large models on many GPUs.
//CUDAʹ��memory pinned����,ʹ�÷�����ڴ治�ᱻ�ͷţ��������Լ���CPU��GPU���ݵĴ����ٶ�
inline void CaffeMallocHost(void** ptr, size_t size, bool* use_cuda) {
#ifndef CPU_ONLY
  if (Caffe::mode() == Caffe::GPU) {
    CUDA_CHECK(cudaMallocHost(ptr, size));		//GPU��ʹ��cuda�����ڴ�
    *use_cuda = true;
    return;
  }
#endif
  *ptr = malloc(size);			//���ֻ����cpu����c��malloc�����ڴ�
  *use_cuda = false;
  CHECK(*ptr) << "host allocation of size " << size << " failed";
}
//�ͷ��ڴ�
inline void CaffeFreeHost(void* ptr, bool use_cuda) {
#ifndef CPU_ONLY
  if (use_cuda) {
    CUDA_CHECK(cudaFreeHost(ptr));
    return;
  }
#endif
  free(ptr);
}


/**
 * @brief Manages memory allocation and synchronization between the host (CPU)
 *        and device (GPU).
 *
 * TODO(dox): more thorough description.
 */
class SyncedMemory {
 public:
  //���캯������ʼ������
  SyncedMemory()
      : cpu_ptr_(NULL), gpu_ptr_(NULL), size_(0), head_(UNINITIALIZED),
        own_cpu_data_(false), cpu_malloc_use_cuda_(false), own_gpu_data_(false),
        gpu_device_(-1) {}
  explicit SyncedMemory(size_t size)
      : cpu_ptr_(NULL), gpu_ptr_(NULL), size_(size), head_(UNINITIALIZED),
        own_cpu_data_(false), cpu_malloc_use_cuda_(false), own_gpu_data_(false),
        gpu_device_(-1) {}
  ~SyncedMemory();		//��������������CaffeFreeHost�ͷ��ڴ�


  const void* cpu_data();	//���cpu����ָ��
  void set_cpu_data(void* data);		//����cpu��������
  const void* gpu_data();	//���gpu����ָ��
  void set_gpu_data(void* data);    //����gpu��������
  void* mutable_cpu_data(); //��ÿɸ��ĵ�cpu����ָ��
  void* mutable_gpu_data(); //��ÿɸ��ĵ�gpu����ָ��
  enum SyncedHead { UNINITIALIZED, HEAD_AT_CPU, HEAD_AT_GPU, SYNCED };
  SyncedHead head() { return head_; }
  size_t size() { return size_; }

  //�첽ͬ������
#ifndef CPU_ONLY
  void async_gpu_push(const cudaStream_t& stream);
#endif

 private:
  void to_cpu();		//״̬ת�ƺ���
  void to_gpu();		//״̬ת�ƺ���
  void* cpu_ptr_;		//cpu�ڴ�ָ��
  void* gpu_ptr_;		//gpu�Դ�ָ��
  size_t size_;		//�ڴ��С
  SyncedHead head_;	//����״̬
  bool own_cpu_data_;		//�����ǣ��Ƿ�ʹ�õ����Լ���cpu����
  bool cpu_malloc_use_cuda_;
  bool own_gpu_data_;		//�����ǣ��Ƿ�ʹ�õ����Լ���gpu����
  int gpu_device_;			//gpu�豸

  DISABLE_COPY_AND_ASSIGN(SyncedMemory);
};  // class SyncedMemory

}  // namespace caffe

#endif  // CAFFE_SYNCEDMEM_HPP_
