#include <iostream>
#include <vector>
#include <cstring>
#include <sndfile.h>
#include <cmath>
#include <random>
#include <chrono>
using namespace std;

#define DF 0.01
#define F0 0.5
#define N 1
#define COEFF_SIZE1 10
#define COEFF_SIZE2 2
#define COEFF_SIZE3 3
#define BPF_OUTPUT "output[BPF][Serial].wav"
#define NF_OUTPUT "output[NF][Serial].wav"
#define FIRF_OUTPUT "output[FIRF][Serial].wav"
#define IIRF_OUTPUT "output[IIRF][Serial].wav"

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

void Notch_Filter(vector<float> data, float f0, int n, vector<float>& filtered)
{
    for(int i = 0; i < data.size(); i++) {
        float hf = 1 / (pow(data[i] / f0, 2 * n) + 1);

        filtered[i] = hf;
    }
}

void Finite_Impulse_Response_Filter(vector<float> data, vector<float> h, vector<float>& filtered)
{
    for(int n = 0; n < data.size(); n++) {
        float temp = 0.0;

        for(int k = 0; k < h.size(); k++) {
            if(n < k)
                break;
            
            temp += h[k] * data[n - k];
        }

        filtered[n] = temp;
    }
}

void Infinite_Impulse_Response_Filter(vector<float> data, vector<float> a, vector<float> b, vector<float>& filtered)
{
    filtered[0] = 0.0;

    for(int n = 0; n < data.size(); n++) {
        float temp = 0.0;

        for(int k = 0; k < a.size(); k++) {
            if(n < k)
                break;
            
            temp += a[k] * data[n - k];
        }

        for(int k = 1; k < b.size(); k++) {
            if(n < k)
                break;

            temp += b[k] * filtered[n - k];
        }

        filtered[n] = temp;
    }
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

    vector<float> filtered(data.size());

    auto start_BPF = chrono::high_resolution_clock::now();
    Band_Pass_Filter(data, DF, filtered);
    auto end_BPF = chrono::high_resolution_clock::now();
    write_wav_file(BPF_OUTPUT, filtered, fileInfo);

    auto start_NF = chrono::high_resolution_clock::now();
    Notch_Filter(data, F0, N, filtered);
    auto end_NF = chrono::high_resolution_clock::now();
    write_wav_file(NF_OUTPUT, filtered, fileInfo);

    auto start_FIRF = chrono::high_resolution_clock::now();
    Finite_Impulse_Response_Filter(data, h, filtered);
    auto end_FIRF = chrono::high_resolution_clock::now();
    write_wav_file(FIRF_OUTPUT, filtered, fileInfo);

    auto start_IIRF = chrono::high_resolution_clock::now();
    Infinite_Impulse_Response_Filter(data, a, b, filtered);
    auto end_IIRF = chrono::high_resolution_clock::now();
    write_wav_file(IIRF_OUTPUT, filtered, fileInfo);

    show_result(end_read - start_read, end_BPF - start_BPF, end_NF - start_NF, end_FIRF - start_FIRF, end_IIRF - start_IIRF, start_run);

    return 0;
}