#pragma once

#include <memory>

// https://stackoverflow.com/a/35866966/13830772

template <class T>
struct poly_const_iterator
{
    using value_type = T;

    struct concept_ {
        virtual void next(int n) = 0;
        virtual const value_type& deref() const = 0;
        virtual bool equal(const void* other) const = 0;
        virtual std::unique_ptr<concept_> clone() const = 0;
        virtual const std::type_info& type() const = 0;
        virtual const void* address() const = 0;
        virtual ~concept_() = default;
    };

    template<class Iter>
    struct model : concept_
    {
        Iter _iter;

        model(Iter iter) : _iter(iter) {}

        void next(int n) override { _iter = std::next(_iter, n); }
        const value_type& deref() const override { return *_iter; }
        bool equal(const void* rp) const override { return _iter == static_cast<const model*>(rp)->_iter; }
        std::unique_ptr<concept_> clone() const override { return std::make_unique<model>(*this); }
        const std::type_info& type() const override { return typeid(_iter); }
        const void* address() const override { return this; }
    };

    std::unique_ptr<concept_> _impl;

public:
    template<class Iter>
    poly_const_iterator(Iter iter) : _impl(std::make_unique<model<Iter>>(iter)) {};

    poly_const_iterator(const poly_const_iterator& r) : _impl(r._impl->clone()) {};

    const value_type& operator*() const { return _impl->deref(); }

    poly_const_iterator& operator+=(int n) {
        _impl->next(n);
        return *this;
    }

    poly_const_iterator& operator++() { return *this += 1; }

    bool operator==(const poly_const_iterator& r) const {
        return _impl->type() == r._impl->type()
            and _impl->equal(r._impl->address());
    }

    bool operator!=(const poly_const_iterator& r) const { return not(*this == r); }
};
