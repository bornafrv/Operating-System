#include <iostream>
#include <vector>
#include <cstring>
#include <sndfile.h>
#include <cmath>
#include <random>
#include <chrono>
#include <pthread.h>
using namespace std;

#define DF 0.01
#define F0 0.5
#define N 1
#define COEFF_SIZE1 10
#define COEFF_SIZE2 2
#define COEFF_SIZE3 3
#define NUM_OF_THREADS_NF 2
#define NUM_OF_THREADS_FIRF 8
#define NUM_OF_THREADS_IIRF 10
#define BPF_OUTPUT "output[BPF][Parallel].wav"
#define NF_OUTPUT "output[NF][Parallel].wav"
#define FIRF_OUTPUT "output[FIRF][Parallel].wav"
#define IIRF_OUTPUT "output[IIRF][Parallel].wav"

struct Thread_Data {
    vector<float> data;
    vector<float> filtered;
    int start_index;
    int end_index;
    float df;
    float f0;
    float n;
    vector<float> h;
    vector<float> a;
    vector<float> b;
};

void read_wav_file(string inputFile, vector<float>& data, SF_INFO& fileInfo)
{
    SNDFILE* inFile = sf_open(inputFile.c_str(), SFM_READ, &fileInfo);

    if(!inFile) {
        cerr << "Error opening input file: " << sf_strerror(NULL) << endl;
        exit(1);
    }

    data.resize(fileInfo.frames * fileInfo.channels);

    sf_count_t numFrames = sf_readf_float(inFile, data.data(), fileInfo.frames);

    if(numFrames != fileInfo.frames) {
        cerr << "Error reading frames from file." << endl;
        sf_close(inFile);
        exit(1);
    }

    sf_close(inFile);
    cout << "Successfully read " << numFrames << " frames from " << inputFile << endl;
}

void write_wav_file(string outputFile, vector<float> data, SF_INFO fileInfo)
{
    sf_count_t originalFrames = fileInfo.frames;

    SNDFILE* outFile = sf_open(outputFile.c_str(), SFM_WRITE, &fileInfo);

    if(!outFile) {
        cerr << "Error opening output file: " << sf_strerror(NULL) << endl;
        exit(1);
    }

    sf_count_t numFrames = sf_writef_float(outFile, data.data(), originalFrames);
    
    if(numFrames != originalFrames) {
        cerr << "Error writing frames to file." << endl;
        sf_close(outFile);
        exit(1);
    }

    sf_close(outFile);
    cout << "Successfully wrote " << numFrames << " frames to " << outputFile << endl;
}

vector<float> generate_coefficient(int size)
{
    vector<float> h;

    random_device device;
    mt19937 gen(device());
    uniform_real_distribution<> dis(0.0, 1.0);

    for(int i = 0; i < size; i++) {
        h.push_back(dis(gen));
    }

    return h;
}

void Band_Pass_Filter(vector<float> data, float df, vector<float>& filtered)
{
    float df2 = df * df;

    for(int i = 0; i < data.size(); i++) {
        float f2 = data[i] * data[i];
        float hf = f2 / (f2 + df2);
        
        filtered[i] = hf;
    }
}

void* Notch_Filter(void* parameters)
{
    Thread_Data* data = (Thread_Data*) parameters;

    for(int i = data->start_index; i < data->end_index; i++) {
        float hf = 1 / (pow(data->data[i] / data->f0, 2 * data->n) + 1);

        data->filtered[i] = hf;
    }

    pthread_exit(0);
}

void* Finite_Impulse_Response_Filter(void* parameters)
{
    Thread_Data* data = (Thread_Data*) parameters;

    for(int n = data->start_index; n < data->end_index; n++) {
        float temp = 0.0;

        for(int k = 0; k < data->h.size(); k++) {
            if(n < k)
                break;
            
            temp += data->h[k] * data->data[n - k];
        }

        data->filtered[n] = temp;
    }

    pthread_exit(0);
}

void* Infinite_Impulse_Response_Filter(void* parameters)
{
    Thread_Data* data = (Thread_Data*) parameters;

    data->filtered[0] = 0.0;

    for(int n = data->start_index; n < data->end_index; n++) {
        float temp = 0.0;

        for(int k = 0; k < data->a.size(); k++) {
            if(n < k)
                break;
            
            temp += data->a[k] * data->data[n - k];
        }

        for(int k = 1; k < data->b.size(); k++) {
            if(n < k)
                break;

            temp += data->b[k] * data->filtered[n - k];
        }

        data->filtered[n] = temp;
    }

    pthread_exit(0);
}

void show_result(auto duration_read, auto duration_BPF, auto duration_NF, auto duration_FIRF, auto duration_IIRF, auto start_run)
{
    auto time_read = chrono::duration_cast<chrono::duration<float, milli>>(duration_read).count();
    auto time_BPF = chrono::duration_cast<chrono::duration<float, milli>>(duration_BPF).count();
    auto time_NF = chrono::duration_cast<chrono::duration<float, milli>>(duration_NF).count();
    auto time_FIRF = chrono::duration_cast<chrono::duration<float, milli>>(duration_FIRF).count();
    auto time_IIRF = chrono::duration_cast<chrono::duration<float, milli>>(duration_IIRF).count();
    auto time_run = chrono::duration_cast<chrono::duration<float, milli>>(chrono::high_resolution_clock::now() - start_run).count();

    cout << "Read: " << time_read << " ms\n" << "Band Pass Filter: " << time_BPF << " ms\n" << "Notch Filter: " << time_NF << " ms\n" <<
            "FIR Filter: " << time_FIRF << " ms\n" << "IIR Filter: " << time_IIRF << " ms\n" << "Execution: " << time_run << " ms" << endl;
}

void create_threads(vector<float> data, vector<float>& filtered, float df, float f0, float n, vector<float> h, vector<float> a, vector<float> b,
                    void* (*filter) (void*), int num_of_threads)
{
    int threads_data_size = data.size() / num_of_threads;
    vector<pthread_t> threads (num_of_threads);
    vector<Thread_Data> thread_data (num_of_threads);

    for (int i = 0; i < num_of_threads; i++) {
        int start_index = i * threads_data_size;
        int end_index = (i == num_of_threads - 1) ? data.size() : start_index + threads_data_size;

        thread_data[i] = {data, filtered, start_index, end_index, df, f0, n, h, a, b};

        pthread_create(&threads[i], NULL, filter, &thread_data[i]);
    }

    for (int i = 0; i < num_of_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char* argv[])
{
    auto start_run = chrono::high_resolution_clock::now();

    if(argc != 2)
        cerr << "Error getting arguments." << endl;

    string inputFile = argv[1];
    vector<float> data;
    SF_INFO fileInfo;

    vector<float> h = generate_coefficient(COEFF_SIZE1);
    vector<float> a = generate_coefficient(COEFF_SIZE2);
    vector<float> b = generate_coefficient(COEFF_SIZE3);

    memset(&fileInfo, 0, sizeof(fileInfo));

    auto start_read = chrono::high_resolution_clock::now();
    read_wav_file(inputFile, data, fileInfo); 
    auto end_read = chrono::high_resolution_clock::now();

    vector<float> filtered (data.size());

    auto start_BPF = chrono::high_resolution_clock::now();
    Band_Pass_Filter(data, DF, filtered);
    auto end_BPF = chrono::high_resolution_clock::now();
    write_wav_file(BPF_OUTPUT, filtered, fileInfo);

    auto start_NF = chrono::high_resolution_clock::now();
    create_threads(data, filtered, DF, F0, N, h, a, b, Notch_Filter, NUM_OF_THREADS_NF);
    auto end_NF = chrono::high_resolution_clock::now();
    write_wav_file(NF_OUTPUT, filtered, fileInfo);

    auto start_FIRF = chrono::high_resolution_clock::now();
    create_threads(data, filtered, DF, F0, N, h, a, b, Finite_Impulse_Response_Filter, NUM_OF_THREADS_IIRF);
    auto end_FIRF = chrono::high_resolution_clock::now();
    write_wav_file(FIRF_OUTPUT, filtered, fileInfo);

    auto start_IIRF = chrono::high_resolution_clock::now();
    create_threads(data, filtered, DF, F0, N, h, a, b, Infinite_Impulse_Response_Filter, NUM_OF_THREADS_IIRF);
    auto end_IIRF = chrono::high_resolution_clock::now();
    write_wav_file(IIRF_OUTPUT, filtered, fileInfo);

    show_result(end_read - start_read, end_BPF - start_BPF, end_NF - start_NF, end_FIRF - start_FIRF, end_IIRF - start_IIRF, start_run);

    return 0;
}