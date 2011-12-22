//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#ifndef EL_SPTR_HPP
#define EL_SPTR_HPP


namespace eldb {

struct Object {
  int ptr_ref;
  Object(): ptr_ref( 0 ) {}
};


template <typename T>
struct Ptr {

  Ptr(): ptr( 0 ) {
  }

  Ptr( T* ptr_ ): ptr( ptr_ ) {
    incRef();
  };

  Ptr( const Ptr<T> &o ): ptr( o.ptr ) {
    incRef();
  };

  ~Ptr() {
    decRef();
  }

  Ptr<T> &operator=( const Ptr<T> &o ) {
    decRef();
    ptr = o.ptr;
    incRef();

    return *this;
  }

  Ptr<T> &operator=( const T *ptr_ ) {
    decRef();
    ptr = ptr_;
    incRef();

    return *this;
  }

  T &operator*() const  {
    return *ptr;
  }

  T *operator->() const {
    return ptr;
  }

  T *get() const {
    return ptr;
  }

  void decRef() {
    if ( ptr ) {
      --ptr->ptr_ref;
      if ( 0 == ptr->ptr_ref ) {
        delete ptr;
      }
    }
  }

  void incRef() {
    if ( ptr ) {
      ++ptr->ptr_ref;
    }
  }

  private:
    template<typename U>
    friend class Ptr;

    T *ptr;
};

} //namespace eldb


#endif
