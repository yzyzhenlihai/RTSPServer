#pragma once 
#include<functional>
#include<mutex>
#include<queue>
#include<condition_variable>
#include"Thread.h"
class ThreadPool{

public:
    class Task{
    public:
        Task():mTaskCallback(nullptr){}
        void setTaskCallback(std::function<void()> cb){
            mTaskCallback = cb;
        }
        void handleTask(){
            if(mTaskCallback){
                mTaskCallback();
            }
        }
    private:    
        std::function<void()> mTaskCallback;
    };
    explicit ThreadPool(int threadNum);
    ~ThreadPool();
    static ThreadPool* createNew(int threadNum);
public:
    void addTask(Task& task);
private:
    void createThreads();
    void cancelThreads();
    void loop();
private:
    int mThreadNum;
    std::queue<Task> mTaskQueue;
    std::vector<Thread> mThreads;
    std::mutex mMtx;
    std::condition_variable mCon; 
    bool mQuit;
    
};