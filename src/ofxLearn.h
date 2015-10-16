#pragma once

#include "ofMain.h"
#ifdef TARGET_WIN32
    #include <Synchapi.h>
#include <Windows.h>
#endif
#include "dlib/svm.h"
#include "dlib/mlp.h"
#include "dlib/svm/svm_threaded.h"
#include "dlib/matrix/matrix_abstract.h"

#include "dlib/statistics/statistics.h"



// dlib examples
// http://dlib.net/mlp_ex.cpp.html
// http://dlib.net/svr_ex.cpp.html
// http://dlib.net/multiclass_classification_ex.cpp.html


// TODO
//  x mlp
//  x svr
//  x multiclass svm
//  x cross validation
//  x grid parameter search
//  - pca, svd
//  - sample from gaussian (http://dlib.net/3d_point_cloud_ex.cpp.html)



// sample type
typedef dlib::matrix<double, 0, 1>                  sample_type;

// kernel types
typedef dlib::radial_basis_kernel<sample_type>      rbf_kernel_type;
typedef dlib::polynomial_kernel<sample_type>        poly_kernel_type;

// trainer types
typedef dlib::any_trainer<sample_type>              any_trainer;
typedef dlib::one_vs_one_trainer<any_trainer>       ovo_trainer;
typedef dlib::mlp::kernel_1a_c                      mlp_trainer_type;


class ofxLearn
{
public:
    ofxLearn() { }
    virtual ~ofxLearn() { }
    void svd();
    
    virtual void train() { }
    
    virtual void saveModel(string path) { }
    virtual void loadModel(string path) { }
    
    inline sample_type vectorToSample(vector<double> sample_);
};



class ofxLearnSupervised : public ofxLearn
{
public:
    ofxLearnSupervised() : ofxLearn() {}
    
    void addTrainingInstance(vector<double> sample, double label);
    void addSample(sample_type sample, double label);
    void clearTrainingInstances();
    
<<<<<<< HEAD
	virtual double predict(vector<double> sample) { return 0.0; }
=======
    virtual double predict(vector<double> & sample) { }
    virtual double predict(sample_type & sample) { }
>>>>>>> a483d636bb83e5114542dc2adcd0573a93987b88
    
protected:

    vector<sample_type> samples;
    vector<double> labels;
    
};

class ofxLearnUnsupervised : public ofxLearn
{
public:
    ofxLearnUnsupervised() : ofxLearn() {}
    
    void addTrainingInstance(vector<double> sample);
    void addSample(sample_type sample);
    void clearTrainingInstances();
    
protected:
    
    vector<sample_type> samples;
    
};


class ofxLearnMLP : public ofxLearnSupervised
{
public:
    ofxLearnMLP();
    ~ofxLearnMLP();
    
    void train();
    double predict(vector<double> & sample);
    double predict(sample_type & sample);
    
    void setHiddenLayers(int hiddenLayers) {this->hiddenLayers = hiddenLayers;}
    void setTargetRmse(float targetRmse) {this->targetRmse = targetRmse;}
    void setMaxSamples(int maxSamples) {this->maxSamples = maxSamples;}

    int getHiddenLayers() {return hiddenLayers;}
    float getTargetRmse() {return targetRmse;}
    int getMaxSamples() {return maxSamples;}

    mlp_trainer_type * getTrainer() {return mlp_trainer;}
    
private:
    
    mlp_trainer_type *mlp_trainer;
    
    int hiddenLayers;
    float targetRmse;
    int maxSamples;
};



class ofxLearnSVR : public ofxLearnSupervised
{
public:
    ofxLearnSVR();
    ~ofxLearnSVR();
    
    void train();
    void trainWithGridParameterSearch();
    double predict(vector<double> & sample);
    double predict(sample_type & sample);
    
private:
    
    dlib::svr_trainer<rbf_kernel_type> trainer;
    dlib::decision_function<rbf_kernel_type> df;
};


class ofxLearnSVM : public ofxLearnSupervised
{
public:
    ofxLearnSVM();
    ~ofxLearnSVM();
    
    void train();
    void trainWithGridParameterSearch();
    
    double predict(vector<double> & sample);
    double predict(sample_type & sample);
    
    void saveModel(string path)
    {
        const char *filepath = path.c_str();
        ofstream fout(filepath, ios::binary);
        dlib::one_vs_one_decision_function<ovo_trainer, dlib::decision_function<rbf_kernel_type> > df2, df3;
        df2 = df;
        serialize(df2, fout);

    }
    
    void loadModel(string path) {
        const char *filepath = path.c_str();
        ifstream fin(filepath, ios::binary);
        dlib::one_vs_one_decision_function<ovo_trainer, dlib::decision_function<rbf_kernel_type> > df2;
        dlib::deserialize(df2, fin);
        df = df2;
    }

private:
    
    ovo_trainer trainer;
    
    //dlib::svm_nu_trainer<poly_kernel_type> poly_trainer;
    dlib::krr_trainer<rbf_kernel_type> rbf_trainer;
    dlib::one_vs_one_decision_function<ovo_trainer> df;
};


class ofxLearnKMeans : public ofxLearnUnsupervised
{
public:
    ofxLearnKMeans();
    
    int getNumClusters() {return numClusters;}
    void setNumClusters(int numClusters);
    void train();
    vector<int> & getClusters() {return clusters;}

private:
    vector<int> clusters;
    int numClusters;
};




////////////////////////

class ofxLearnThreaded : public ofxLearn, public ofThread
{
public:
    ofxLearnThreaded() : ofxLearn()
    {
        trained = false;
    }
    
    ~ofxLearnThreaded() {
        //finishedTrainingE.clear();
        finishedTrainingE.disable();
    }
    
    void beginTraining()
    {
        trained = false;
        startThread();
    }
    
    template <typename L, typename M>
    void beginTraining(L *listener, M method);
    
    bool isTrained() {return trained;}
    
private:
    
    void threadedFunction();
    virtual void threadedTrainer() {};
    
    bool trained;
    ofEvent<void> finishedTrainingE;
};

template <typename L, typename M>
void ofxLearnThreaded::beginTraining(L *listener, M method)
{
    ofAddListener(finishedTrainingE, listener, method);
    beginTraining();
}

class ofxLearnMLPThreaded : public ofxLearnMLP, public ofxLearnThreaded {
    void threadedTrainer() {ofxLearnMLP::train();}
};

class ofxLearnSVRThreaded : public ofxLearnSVR, public ofxLearnThreaded {
    void threadedTrainer() {ofxLearnSVR::train();}
};

class ofxLearnSVMThreaded : public ofxLearnSVM, public ofxLearnThreaded {
    void threadedTrainer() {ofxLearnSVM::train();}
};

class ofxLearnKMeansThreaded : public ofxLearnKMeans, public ofxLearnThreaded {
    void threadedTrainer() {ofxLearnKMeans::train();}
};

