#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>
#include <iostream>

template <class T>
class RingBuffer
{
public:
    ~RingBuffer();
    RingBuffer(int n = 0);

    void setSize(int n);
    int len() { return size; }
    T& operator[](int i);
    void write(T* input, int n);

    T* get();

private:
    T* buf;
    int pointer = 0,    // represents the next space to write to, loops back to 0 when greater than size
        size = 0;
    T* tBuf = nullptr;
};

template <class T>
RingBuffer<T>::~RingBuffer<T>() {
    if (buf != nullptr) delete[] buf;
    if (tBuf != nullptr) delete[] tBuf;
}

template <class T>
RingBuffer<T>::RingBuffer(int n) {
    setSize(n);
}

template <class T>
void RingBuffer<T>::setSize(int n) {
    T *temp = new T[n];
    memset(temp, 0, sizeof(float) * n);
    if (size != 0) {
        memcpy(temp, buf, sizeof(T) * size);
        delete[] buf;
    }
    size = n;
    buf = temp;
}

template <class T>
T& RingBuffer<T>::operator[](int i) {
    return i < size && i >= 0 ? buf[(i + pointer) % size] : buf[0];
}

template <class T>
void RingBuffer<T>::write(T* input, int n) {
    if (size == 0) return;
    for (int i = 0; i < n; i++, pointer++) {
        pointer = pointer % size;
        buf[pointer] = input[i];
    }
}

template <class T>
T* RingBuffer<T>::get() {
    if (size == 0) return nullptr;
    if (tBuf != nullptr) delete[] tBuf;
    tBuf = new T[size];
    memcpy(tBuf, &buf[pointer], sizeof(T) * (size - pointer));
    memcpy(tBuf + size-pointer, &buf[0], sizeof(T) * (pointer));
    return tBuf;
}


#endif // RINGBUFFER_H
