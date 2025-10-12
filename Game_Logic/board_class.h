#pragma once
#include <cassert>

//honestly, this file was mostly written by chatgpt.
template<typename T>
class Array3D {
private:
    std::vector<T> data;
    int width, height, depth;
    

    inline int index(int x, int y, int z) const {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        assert(z >= 0 && z < depth);
        return x * height * depth + y * depth + z;
    }


public:
    Array3D() = default;

    Array3D(int w, int h, int d) : width(w), height(h), depth(d), data(w* h* d) {}

    T& at(int x, int y, int z) {
        return data[index(x, y, z)];
    }

    const T& at(int x, int y, int z) const {
        return data[index(x, y, z)];
    }

    void resize(int w, int h, int d) {
        width = w; height = h; depth = d;
        data.resize(w * h * d);
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getDepth() const { return depth; }

    void fill(const T& value) {
        std::fill(data.begin(), data.end(), value);
    }
    T& operator()(int x, int y, int z) {
        return data[index(x, y, z)];
    }
    const T& operator()(int x, int y, int z) const {
        return data[index(x, y, z)];
    }

};
