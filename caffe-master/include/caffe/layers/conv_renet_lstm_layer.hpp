#pragma once 

#include <vector>

#include "caffe/blob.hpp"
#include "caffe/layer.hpp"
#include "caffe/proto/caffe.pb.h"
#include "caffe/layers/renet_lstm_layer.hpp"

namespace caffe
{
	template<typename Dtype>
	class ConvolutionReNetLSTMLayer : public Layer<Dtype>
	{
	public:
		explicit ConvolutionReNetLSTMLayer(const LayerParameter& param) : Layer<Dtype>(param){}
		
		
		virtual void LayerSetUp(const vector<Blob<Dtype>*>& bottom,
			const vector<Blob<Dtype>*>& top);
		
		virtual void Reshape(const vector<Blob<Dtype>*>& bottom,
			const vector<Blob<Dtype>*>& top);

		virtual inline int ExactNumBottomBlobs() const {
			return 1;
		}
		virtual inline int ExactNumTopBlobs() const {
			return 1;
		}
		virtual inline bool EqualNumBottomTopBlobs() const { return true; }
		virtual inline const char* type() const { return "ConvolutionReNetLSTM"; }

	protected:
		virtual void Forward_cpu(const vector<Blob<Dtype>*>& bottom,const vector<Blob<Dtype>*>& top);
		//virtual void Forward_gpu(const vector<Blob<Dtype>*>& bottom,const vector<Blob<Dtype>*>& top);
		virtual void Backward_cpu(const vector<Blob<Dtype>*>& top,const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);
		//virtual void Backward_gpu(const vector<Blob<Dtype>*>& top,const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);



		void CopyData(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top, int step_id);
		void ComputeGateData();
		void ComputeCellData(const vector<Blob<Dtype>*>& top,int step_id);




		int batch_size_;	//ÿ�����������������
		int T_;//���г��ȣ�num/batch_size��


		int height_;			//�����ͼ���С
		int width_;
		int channels_;		//���ͼ���ͨ�����������ܱ�2����

		vector<shared_ptr<Blob<Dtype> > > data_;	//��bottom�����ݸ����ķ�ͳһ����һ��renetѵ��
		vector<shared_ptr<Blob<Dtype> > > h_;		//��h(t-1)�����ݸ����ķ�ͳһ����һ��renetѵ��
		vector<shared_ptr<ReNetLSTMLayer<Dtype>>> wx_wh_; // [0]Ϊw*x   [1] Ϊw*h

		vector<shared_ptr<Blob<Dtype>>>	gate_;			//ft it  ot gt
		Blob<Dtype> cell_;


		//��ʱ����
		vector<shared_ptr<Blob<Dtype>>>	gate_wh_;		

	
	private:



	};
}