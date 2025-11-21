#include"ThreadPool.h"



ThreadPool::ThreadPool(int threadNum):
    mThreadNum(threadNum),
    mQuit(false),
    mThreads(threadNum)
{
    createThreads();
}

ThreadPool::~ThreadPool(){
    cancelThreads();
}

ThreadPool* ThreadPool::createNew(int threadNum){
    return new ThreadPool(threadNum);
}

void ThreadPool::createThreads(){
    std::unique_lock<std::mutex> locker(mMtx);
    for(auto& thread : mThreads){
        thread.start(std::bind(&ThreadPool::loop, this));
    }
}   


void ThreadPool::loop(){
    while(true){
        Task task;
        {
            std::unique_lock<std::mutex> locker(mMtx);
            // 线程会一直在此沉睡，直到 "队列不为空" 或 "被通知退出"
            mCon.wait(locker, [this]{
                return !mTaskQueue.empty() || mQuit;
            });
            if(mQuit && mTaskQueue.empty()){
                return;
            }
            task = mTaskQueue.front();
            mTaskQueue.pop();
        }
        task.handleTask();
    }
}

void ThreadPool::cancelThreads(){
    std::unique_lock<std::mutex> locker(mMtx);
    mQuit = true;
    mCon.notify_all();
    for(auto& thread : mThreads){
        thread.join(); // join的目的是在ThreadPool销毁前，先阻塞等待所有线程执行完毕，避免ThreadPool先销毁
    }
    mThreads.clear();
}

void ThreadPool::addTask(Task& task){
    std::unique_lock<std::mutex> locker(mMtx);
    mTaskQueue.push(task);
    mCon.notify_one();
}

