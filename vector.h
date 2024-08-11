#ifndef CONTAINERS_VECTOR_H
#define CONTAINERS_VECTOR_H

#include <memory>
#include <cmath>

namespace containers {
    template<class T, bool isConst>
    class VectorIterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<isConst, const T&, T&>;
        using pointer = std::conditional_t<isConst, const T*, T*>;
        using iterator_category = std::contiguous_iterator_tag;

    private:
        pointer _iterator;

    public:
        VectorIterator() : _iterator(nullptr) {}
        explicit VectorIterator(pointer ptr) : _iterator(ptr) {}
        VectorIterator(const VectorIterator &iter) : _iterator(iter._iterator) {}

        VectorIterator& operator=(const VectorIterator &iter) { _iterator = iter._iterator; return *this; }

        reference operator*() const { return *_iterator; }
        pointer operator->() const { return _iterator; }

        VectorIterator operator+(difference_type n) const { return VectorIterator(_iterator + n); }
        VectorIterator operator-(difference_type n) const { return VectorIterator(_iterator - n); }
        VectorIterator& operator+=(difference_type n) const { _iterator += n; return *this; }
        VectorIterator& operator-=(difference_type n) const { _iterator += n; return *this; }
        VectorIterator& operator++() { _iterator += 1; return *this; }
        VectorIterator& operator--() { _iterator -= 1; return *this; }
        VectorIterator operator++(int) { VectorIterator tmp = *this; ++_iterator; return tmp; }
        VectorIterator operator--(int) { VectorIterator tmp = *this; --_iterator; return tmp; }

        reference operator[](difference_type n) const { return *(_iterator + n); }

        bool operator==(const VectorIterator& other) const { return _iterator == other._iterator; }
        bool operator!=(const VectorIterator& other) const { return _iterator != other._iterator; }
        bool operator<(const VectorIterator& other) const { return _iterator < other._iterator; }
        bool operator<=(const VectorIterator& other) const { return _iterator <= other._iterator; }
        bool operator>(const VectorIterator& other) const { return _iterator > other._iterator; }
        bool operator>=(const VectorIterator& other) const { return _iterator >= other._iterator; }
        
    };

    template<class T, class Allocator = std::allocator<T>>
    class vector {
    public:
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = std::allocator_traits<Allocator>::pointer;
        using const_pointer = std::allocator_traits<Allocator>::const_pointer;
        using iterator = VectorIterator<T, false>;
        using const_iterator = VectorIterator<T, true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    private:
        pointer _data;
        size_type _size;
        size_type _capacity;
        allocator_type _allocate;

    private:
        template<class U, class... Args>
        void construct(U* p, Args&&... args) { ::new((void*)p) U(std::forward<Args>(args)...); }
        template<class U>
        void destroy(U* p) { p->~U(); }

    public:
        vector() : _data(nullptr), _size(0), _capacity(0) {}

        ~vector() {
            if constexpr (!std::is_trivial_v<value_type>)
                for (size_type i = 0; i < _size; ++i)
                    destroy(_data[i]);
            _allocate.deallocate(_data, _capacity);
        }

        explicit vector(size_type size, const value_type &value = value_type()) : _size(size), _capacity(size) {
            _data = _allocate.allocate(_capacity);
            // TODO: протестить
//            if constexpr (std::is_trivial_v<value_type>)
//            {
//                std::fill(_data, _data + _size, value);
//            }
//            else
            {
                size_type i = 0;
                try {
                    for (; i < _size; ++i)
                        construct(_data + i, value);
                }
                catch (...) {
                    for (size_type j = 0; j < i; ++j)
                        destroy(_data + j);
                    _allocate.deallocate(_data, _capacity);
                    throw;
                }

            }
        }

        vector(const std::initializer_list<value_type> &list) : _size(list.size()), _capacity(list.size()) {
            _data = _allocate.allocate(_capacity);
            auto iter = list.begin();
            for (size_type i = 0; i < _size; ++i, ++iter)
                construct(_data + i, *iter);
        }

        vector(const vector &rhs) : _size(rhs._size), _capacity(rhs._capacity) {
            _data = _allocate.allocate(rhs._capacity);
            auto iter = rhs.begin();
            for (size_type i = 0; i < _capacity; ++i, ++iter)
                construct(_data + i, *iter);
        }

        vector(const vector &&rhs) noexcept: _data(rhs._data), _size(rhs._size), _capacity(rhs._capacity) {
            rhs._data = nullptr;
            rhs._size = 0;
            rhs._capacity = 0;
        }

        vector &operator=(const vector &rhs) {
            if (this != &rhs) {
                ~vector();
                vector(rhs);
            }
            return *this;
        }

        vector &operator=(vector &&rhs) noexcept {
            if (this == &rhs)
                return *this;
            std::swap(_data, rhs._data);
            rhs._size = 0;
            rhs._capacity = 0;

            return *this;
        }

        constexpr allocator_type get_allocator() const { return _allocate; }

        void assign(size_type count, const_reference value) {
            ~vector();
            vector(count, value);
        }

        template<class InputIt>
        using iterator_category_t = typename std::iterator_traits<InputIt>::iterator_category;

        template<class InputIt, typename S = std::enable_if<std::is_base_of_v<std::input_iterator_tag, iterator_category_t<InputIt>>>>
        void assign(InputIt first, InputIt last) {
            ~vector();
            vector(std::distance(first, last));
            std::copy(first, last, std::back_inserter(_data));
        }

        void assign(std::initializer_list<T> ilist) {
            ~vector();
            _data = _allocate.allocate(_capacity);
            auto iter = ilist.begin();
            for (size_type i = 0; i < _size; ++i, ++iter)
                construct(_data + i, *iter);
        }

        // iterators
        iterator begin() { return iterator(_data); }

        iterator end() { return iterator(_data + _size); }

        const_iterator begin() const { return const_iterator(_data); }

        const_iterator end() const { return const_iterator(_data + _size); }

        const_iterator cbegin() const { return const_iterator(_data); }

        const_iterator cend() const { return const_iterator(_data + _size); }

        reverse_iterator rbegin() { return reverse_iterator(iterator(_data)); }

        reverse_iterator rend() { return reverse_iterator(iterator(_data) + _size); }

        const_reverse_iterator rbegin() const { return const_reverse_iterator(const_iterator(_data)); }

        const_reverse_iterator rend() const { return const_reverse_iterator(const_iterator(_data) + _size); }

        const_reverse_iterator crbegin() const { return const_reverse_iterator(const_iterator(_data)); }

        const_reverse_iterator crend() const { return const_reverse_iterator(const_iterator(_data) + _size); }

        // access
        reference at(size_type pos) {
            if (pos >= _size)
                throw std::out_of_range("pos is out o range of container");
            return *(_data + pos);
        }

        const_reference at(size_type pos) const {
            if (pos >= _size)
                throw std::out_of_range("pos is out o range of container");
            return *(_data + pos);
        }

        reference operator[](size_type pos) { return _data[pos]; }

        const_reference operator[](size_type pos) const { return _data[pos]; }

        reference front() { return *_data; }

        const_reference front() const { return *_data; }

        reference back() { return *(_data + _size); }

        const_reference back() const { return *(_data + _size); }

        pointer data() { return _size == 0 ? nullptr : _data; }

        const_pointer data() const { return _size == 0 ? nullptr : _data; }

        // capacity
        size_type capacity() const noexcept { return _capacity; }

        size_type size() const noexcept { return _size; }

        bool empty() const noexcept {
            if (begin(_data) == end(_data))
                return true;
            return false;
        }

        size_type max_size() const noexcept {
            return std::numeric_limits<difference_type>::max();
        }

        void reserve(size_t new_cap) {
            if (new_cap <= _capacity)
                return;

            if constexpr (std::is_nothrow_move_constructible_v<value_type>)
                reserve_move(new_cap);
            else
                reserve_copy(new_cap);
            _capacity = new_cap;
        }

        void shrink_to_fit() {
            if constexpr (std::is_nothrow_move_constructible_v<value_type>)
                reserve_move(_size);
            else
                reserve_copy(_size);
        }


    private:
        void reserve_copy(size_t new_cap) {
            auto temp = _allocate.allocate(new_cap);
            size_type i = 0;
            try {
                for (; i < _size; ++i)
                    construct(temp + i, _data[i]);
            }
            catch (...) {
                for (size_type j = 0; j < i; ++j)
                    destroy(temp + j);
                _allocate.deallocate(temp, new_cap);
                throw;
            }
            for (size_type j = 0; j < _size; ++j)
                destroy(_data + j);
            _allocate.deallocate(_data, _capacity);
            std::swap(temp, _data);
        }

        void reserve_move(size_t new_cap) {
            auto temp = _allocate.allocate(new_cap);
            for (size_type i = 0; i < _size; ++i) {
                construct(temp + i, std::move(_data[i]));
                destroy(_data + i);

            }
            _allocate.deallocate(_data, _capacity);
            std::swap(temp, _data);
        }


        // modifiers
        void clear() noexcept {
            for (size_type i = 0; i < _size; ++i)
                destroy(_data + i);
            _size = 0;
        }
    };
}


#endif //CONTAINERS_VECTOR_H
