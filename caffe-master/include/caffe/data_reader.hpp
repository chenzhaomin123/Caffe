#ifndef CAFFE_DATA_READER_HPP_
#define CAFFE_DATA_READER_HPP_

#include <map>
#include <string>
#include <vector>

#include "caffe/common.hpp"
#include "caffe/internal_thread.hpp"
#include "caffe/util/blocking_queue.hpp"
#include "caffe/util/db.hpp"

//Body-DataReader������Caffe���ݻ���ĵ�һ�������ݿ�->Datum 

namespace caffe {

/**
 * @brief Reads data from a source to queues available to data layers.
 * A single reading thread is created per source, even if multiple solvers
 * are running in parallel, e.g. for multi-GPU training. This makes sure
 * databases are read sequentially, and that each solver accesses a different
 * subset of the database. Data is distributed to solvers in a round-robin
 * way to keep parallel training deterministic.
 */
class DataReader {
 public:
	 //���캯�������������ݣ�Datumȫ�ǿ�Ԫ�أ��Ҵ�����е�Ӧ����ָ�롣
  explicit DataReader(const LayerParameter& param);
  ~DataReader();

  inline BlockingQueue<Datum*>& free() const {
    return queue_pair_->free_;
  }
  inline BlockingQueue<Datum*>& full() const {
    return queue_pair_->full_;
  }

 protected:
  // Queue pairs are shared between a body and its readers
  //˫������У�������Ч�������Ƿ�����
  class QueuePair {
   public:
    explicit QueuePair(int size);
    ~QueuePair();

    BlockingQueue<Datum*> free_;
    BlockingQueue<Datum*> full_;

  DISABLE_COPY_AND_ASSIGN(QueuePair);
  };

  // A single body is created per source
  //����Դ
  class Body : public InternalThread {
   public:
    explicit Body(const LayerParameter& param);
    virtual ~Body();

   protected:
    void InternalThreadEntry();
    void read_one(db::Cursor* cursor, QueuePair* qp);

    const LayerParameter param_;
    BlockingQueue<shared_ptr<QueuePair> > new_queue_pairs_;

    friend class DataReader;

  DISABLE_COPY_AND_ASSIGN(Body);
  };

  // A source is uniquely identified by its layer name + path, in case
  // the same database is read from two different locations in the net.
  static inline string source_key(const LayerParameter& param) {
    return param.name() + ":" + param.data_param().source();
  }

  const shared_ptr<QueuePair> queue_pair_;
  shared_ptr<Body> body_;
  /*
   *��Body��shared_ptr����map���������ᵼ��ָ���������ԶΪ1��
   *������������׼����Body��map���������ʱ���޷���֪���Ƿ��Ѿ����ͷš�
   *��weak_ptrָ��shared_ptrʱ����������ָ�������������������Ϊ0ʱ�����ɽ����map�������
   *һ��body��ʾһ������Դ��Ҳ���ǿ��ԴӶ������Դ���������
   */
  static map<const string, boost::weak_ptr<DataReader::Body> > bodies_;

DISABLE_COPY_AND_ASSIGN(DataReader);
};

}  // namespace caffe

#endif  // CAFFE_DATA_READER_HPP_
