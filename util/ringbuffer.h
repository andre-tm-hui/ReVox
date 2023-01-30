#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>

template <class T>
class RingBuffer
{
public:
    RingBuffer(int n = 0);

    void setSize(int n);
    int len() { return size; }
    T* operator[](int i);
    void write(float* buf, int n);

    T* get();

private:
    std::vector<T> buf;
    int pointer = 0,    // represents the next space to write to, loops back to 0 when greater than size
        size = 0;
    T* tBuf = nullptr;
};

template <class T>
RingBuffer<T>::RingBuffer(int n) {
    setSize(n);
}

template <class T>
void RingBuffer<T>::setSize(int n) {
    size = n;
    buf.resize(n);
}

template <class T>
T* RingBuffer<T>::operator[](int i) {
    return i < size && i >= 0 ? buf[(i+1) % size] : NULL;
}

template <class T>
void RingBuffer<T>::write(float* buf, int n) {
    if (size == 0) return;
    for (int i = 0; i < n; i++, pointer++) {
        pointer = pointer % size;
        this->buf[pointer] = buf[i];
    }
}

template <class T>
T* RingBuffer<T>::get() {
    if (size == 0) return nullptr;
    if (tBuf != nullptr) delete[] tBuf;
    tBuf = new T[size];
    int startPos = (pointer+1) % size;
    memcpy(tBuf, &buf[startPos], sizeof(T) * (size - startPos));
    memcpy(tBuf + size-startPos, &buf[0], sizeof(T) * (startPos));
    return tBuf;
}


#endif // RINGBUFFER_H
