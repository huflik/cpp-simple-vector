#pragma once

#include <initializer_list>
#include <algorithm>
#include <stdexcept>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) : capacity_(capacity) {}
    size_t GetCapacity() {
        return capacity_;
    }
private:
    size_t capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

 
    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : items_(size), size_(size), capacity_(size) {}

    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        std::fill(this->begin(), this->end(), value);
    }

    SimpleVector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), this->begin());
    }
 
    SimpleVector(const SimpleVector& other) : items_(other.size_), size_(other.size_), capacity_(other.size_) {
        std::copy(other.begin(), other.end(), this->begin());
    }

    SimpleVector(SimpleVector&& other)  noexcept : SimpleVector() {
        swap(other);
    }
 
    SimpleVector(ReserveProxyObj r) : items_(r.GetCapacity()), size_(0), capacity_(r.GetCapacity()) {}

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs)
        {
            delete[] items_.Release();
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs)
        {
            delete[] items_.Release();
            SimpleVector tmp(std::move(rhs));
            swap(tmp);
        }
        return *this;
    }

    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            items_[size_] = item;
            ++size_;
        }
        else {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> resize_vector(new_capacity);
            std::copy(this->begin(), this->end(), resize_vector.Get());
            resize_vector[size_] = item;
            delete[] items_.Release();
            items_.swap(resize_vector);
            ++size_;
            capacity_ = new_capacity;
        }
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            items_[size_] = std::move(item);
            ++size_;
        }
        else {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> resize_vector(new_capacity);
            std::move(this->begin(), this->end(), resize_vector.Get());
            resize_vector[size_] = std::move(item);
            delete[] items_.Release();
            items_.swap(resize_vector);
            ++size_;
            capacity_ = new_capacity;
        }
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        Iterator r;
        if (size_ < capacity_)
        {
            ConstIterator old_end = this->end();
            ++size_;
            std::copy_backward(pos, old_end, this->end());
            r = const_cast<Iterator>(pos);
        }
        else
        {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> new_vector(new_capacity);
            r = std::copy(this->cbegin(), pos, new_vector.Get());
            std::copy_backward(pos, this->cend(), new_vector.Get() + size_ + 1);
            delete[] items_.Release();
            items_.swap(new_vector);
            ++size_;
            capacity_ = new_capacity;
        }
        *r = value;
        return r;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        Iterator r;
        Iterator pos_ = const_cast<Iterator>(pos);
        if (size_ < capacity_)
        {
            Iterator old_end = this->end();
            ++size_;
            std::move_backward(pos_, old_end, this->end());
            r = pos_;
        }
        else
        {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> new_vector(new_capacity);

            r = std::move(this->begin(), pos_, new_vector.Get());
            std::move_backward(pos_, this->end(), new_vector.Get() + size_ + 1);
            delete[] items_.Release();
            items_.swap(new_vector);
            ++size_;
            capacity_ = new_capacity;
        }
        *r = std::move(value);
        return r;
    }

    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        Iterator r = const_cast<Iterator>(pos);
        Iterator begin = const_cast<Iterator>(pos + 1);

        std::move(begin, this->end(), r);
        --size_;
        return r;
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(this->size_, other.size_);
        std::swap(this->capacity_, other.capacity_);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return items_[index];
    }


    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size > size_) {
            if (new_size < capacity_) {
                ArrayPtr<Type> resize_vector(capacity_);
                std::move(this->begin(), this->end(), resize_vector.Get());
                delete[] items_.Release();
                items_.swap(resize_vector);
            }
            else {
                size_t new_capacity = std::max(new_size, 2 * capacity_);
                ArrayPtr<Type> resize_vector(new_capacity);
                std::move(this->begin(), this->end(), resize_vector.Get());
                delete[] items_.Release();
                items_.swap(resize_vector);
                capacity_ = new_capacity;
            }
        }
        size_ = new_size;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_vector(new_capacity);
            if (!IsEmpty()) {
                std::move(this->begin(), this->end(), new_vector.Get());
            }
            delete[] items_.Release();
            items_.swap(new_vector);
            capacity_ = new_capacity;
        }
    }

    Iterator begin() noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get();
    }

    Iterator end() noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get();
    }

    ConstIterator cend() const noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get() + size_;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;

};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() != rhs.GetSize() || !(std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}