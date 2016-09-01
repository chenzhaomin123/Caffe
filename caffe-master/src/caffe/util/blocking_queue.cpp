#include <boost/thread.hpp>
#include <string>

#include "caffe/data_reader.hpp"
#include "caffe/layers/base_data_layer.hpp"
#include "caffe/parallel.hpp"
#include "caffe/util/blocking_queue.hpp"

namespace caffe {

	//�������ߺ��������߳���Ҫ��ͬһ��Դ���в�����ʱ����Ҫ��������
	/*
	*��������
	* 1���������գ���ʱ�����߲������ѣ��ܾ�pop����֮�󣬿��Խ���CPU����Ȩ��
	* 2��������������ʱ�����߲����������ܾ�push����֮�󣬿��Խ���CPU����Ȩ��
	*�����������̣߳����̲߳������������Ҫ�������̼߳���
	*/

template<typename T>
class BlockingQueue<T>::sync {
 public:
  mutable boost::mutex mutex_;						//������
  boost::condition_variable condition_;				//blocking�������뻥�ⲻͬ������Ὣ����̶߳�ͬһ����Դ���첽���в���������һ������ִ�ж��У����еȴ�ִ�С���blocking���ǽ��߳����ߣ�CPU����ʱ����������ơ�
};

template<typename T>
BlockingQueue<T>::BlockingQueue()
    : sync_(new sync()) {
}

template<typename T>
void BlockingQueue<T>::push(const T& t) {
  boost::mutex::scoped_lock lock(sync_->mutex_);		//scoped_lock�������ֲ����������������֮�����̽��������Բ���Ҫ�����Լ��ֶ�����unlock
  queue_.push(t);
  lock.unlock();
  sync_->condition_.notify_one();				//����һ���߳�
}

template<typename T>
bool BlockingQueue<T>::try_pop(T* t) {
  boost::mutex::scoped_lock lock(sync_->mutex_);

  if (queue_.empty()) {
    return false;
  }

  *t = queue_.front();
  queue_.pop();
  return true;
}

template<typename T>
T BlockingQueue<T>::pop(const string& log_on_wait) {
  boost::mutex::scoped_lock lock(sync_->mutex_);

  while (queue_.empty()) {
    if (!log_on_wait.empty()) {
      LOG_EVERY_N(INFO, 1000)<< log_on_wait;
    }
    sync_->condition_.wait(lock);		//ʹ�õ�ǰmutexΪ��ǣ�����cpu����Ȩ
  }

  T t = queue_.front();
  queue_.pop();
  return t;
}

template<typename T>
bool BlockingQueue<T>::try_peek(T* t) {
  boost::mutex::scoped_lock lock(sync_->mutex_);

  if (queue_.empty()) {
    return false;
  }

  *t = queue_.front();
  return true;
}

template<typename T>
T BlockingQueue<T>::peek() {
  boost::mutex::scoped_lock lock(sync_->mutex_);

  while (queue_.empty()) {
    sync_->condition_.wait(lock);
  }

  return queue_.front();
}

template<typename T>
size_t BlockingQueue<T>::size() const {
  boost::mutex::scoped_lock lock(sync_->mutex_);
  return queue_.size();
}

//ʵ������
template class BlockingQueue<Batch<float>*>;
template class BlockingQueue<Batch<double>*>;
template class BlockingQueue<Datum*>;
template class BlockingQueue<shared_ptr<DataReader::QueuePair> >;
template class BlockingQueue<P2PSync<float>*>;
template class BlockingQueue<P2PSync<double>*>;

}  // namespace caffe
