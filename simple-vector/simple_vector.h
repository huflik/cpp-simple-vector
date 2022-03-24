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

    // Конструктор по-умолчанию
    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : items_(size), size_(size), capacity_(size) {}

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        std::fill(this->begin(), this->end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), this->begin());
    }
    // Конструктор копирования
    SimpleVector(const SimpleVector& other) : items_(other.size_), size_(other.size_), capacity_(other.size_) {
        std::copy(other.begin(), other.end(), this->begin());
    }
    // Конструктор перемещения
    SimpleVector(SimpleVector&& other)  noexcept : SimpleVector()  {
        swap(other);
    }
    // Конструктор с заданной вместимостью
    SimpleVector(ReserveProxyObj r) : items_(r.GetCapacity()), size_(0), capacity_(r.GetCapacity()) {}

    // Оператор присваивания копированием
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs)
        {
            delete[] items_.Release();
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    // Оператор присваивания перемещением
    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs)
        {
            delete[] items_.Release();
            SimpleVector tmp(std::move(rhs));
            swap(tmp);   
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
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


    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
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

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }
    
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        Iterator r = const_cast<Iterator>(pos);
        Iterator begin = const_cast<Iterator>(pos + 1);

        std::move(begin, this->end(), r);
        --size_;
        return r;
    }
    
    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(this->size_, other.size_);
        std::swap(this->capacity_, other.capacity_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
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

    // Резервирование вместимость
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


    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        if (size_ == 0 && capacity_ == 0) {
            return nullptr;
        }
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
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