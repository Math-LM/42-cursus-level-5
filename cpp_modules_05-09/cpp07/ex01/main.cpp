#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <stdexcept>

template <typename T>
class Array
{
private:
    T *_data;
    unsigned int _size;

public:
    // Default constructor: empty array
    Array() : _data(NULL), _size(0)
    {
    }

    // Constructor with size parameter
    Array(unsigned int n) : _size(n)
    {
        _data = new T[n]();
    }

    // Copy constructor
    Array(const Array &other) : _size(other._size)
    {
        _data = new T[_size];
        for (unsigned int i = 0; i < _size; i++)
            _data[i] = other._data[i];
    }

    // Assignment operator
    Array &operator=(const Array &other)
    {
        if (this != &other)
        {
            delete[] _data;
            _size = other._size;
            _data = new T[_size];
            for (unsigned int i = 0; i < _size; i++)
                _data[i] = other._data[i];
        }
        return *this;
    }

    // Destructor
    ~Array()
    {
        delete[] _data;
    }

    // Subscript operator - non-const version
    T &operator[](unsigned int index)
    {
        if (index >= _size)
            throw std::out_of_range("Index out of bounds");
        return _data[index];
    }

    // Subscript operator - const version
    const T &operator[](unsigned int index) const
    {
        if (index >= _size)
            throw std::out_of_range("Index out of bounds");
        return _data[index];
    }

    // Size member function
    unsigned int size(void) const
    {
        return _size;
    }
};

#endif