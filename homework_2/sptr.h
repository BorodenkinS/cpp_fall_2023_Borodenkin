template <class T>
class UniquePtr {
public:
    UniquePtr() : _ptr() {}

    UniquePtr(const UniquePtr& o) = delete;
    UniquePtr& operator=(const UniquePtr& o) = delete;
    
    UniquePtr(UniquePtr&& o) : _ptr(o.Release()) {};
    
    UniquePtr& operator=(UniquePtr&& o) 
    { 
        Reset(o.Release());
        return *this;
    }
    
    UniquePtr(T* p) : _ptr(p) {}
    
    T* operator->() {  return _ptr; }
    
    T& operator*() {  return *_ptr; }
    
    ~UniquePtr() {  Delete(_ptr); }
    
private:
    T* _ptr;

    void Delete(T* p)
    {
        if (p != nullptr){
            delete p;
        }
    }

    T* Release()
    {
        T* new_p = _ptr;
        _ptr = nullptr;
        return new_p;
    }

    void Reset(T* new_ptr)
    {
        T* old_ptr = _ptr;
        _ptr = new_ptr;
        if (old_ptr != _ptr){
            Delete(old_ptr);
        }
    }

    T* Get()
    {
        return _ptr;
    }

};


// ================================================
struct RefCntBlock {
    size_t strong, weak;
};

template <class T>
class WeakPtr;

template <class T>
class SharedPtr {   
    friend class WeakPtr<T>;
public:
    SharedPtr() : _ptr(), _counter() {};
    SharedPtr(const SharedPtr& o) : _ptr(o._ptr), _counter(o._counter)
    {
        EnlargeCounter();
    }
    SharedPtr& operator=(const SharedPtr& o)
    {
        if (_counter != o._counter)
        {
            this->~SharedPtr();
        }

        _ptr = o._ptr;
        _counter = o._counter;
        EnlargeCounter();
        return *this;
    }
    
    SharedPtr(SharedPtr&& o) : _ptr(o._ptr), _counter(o._counter)
    {
        EnlargeCounter();
        o.Reset();
    }
    
    SharedPtr& operator=(SharedPtr&& o)
    {
        if (_counter != o._counter)
        {
            this->~SharedPtr();
        }

        _ptr = o._ptr;
        _counter = o._counter;
        EnlargeCounter();
        o.Reset();
        return *this;
    }
    
    SharedPtr(T* p) : _ptr(p), _counter(new RefCntBlock())
    {
        _counter->strong++;
    }
    
    // Implementation below
    SharedPtr(const WeakPtr<T>& o);
    
    // Replaces pointer with nullptr
    void Reset()
    {
        this->~SharedPtr();
        _ptr = nullptr;
        _counter = nullptr;
    }
    
    T* operator->() {  return _ptr;  }
    
    T& operator*() { return *_ptr; }
    
    ~SharedPtr ()
    {
        if (_counter != nullptr)
        {
            _counter->strong--;
            if (_counter->strong == 0)
            {
                delete _ptr;

                if (_counter->weak == 0)
                {
                    delete _counter;
                }
            }
        }
    }
    
private:
    T* _ptr;
    RefCntBlock* _counter;

    void EnlargeCounter()
    {
        if (_counter != nullptr)
        {
            _counter->strong++;
        }
    }

    T* Get()
    {
        return _ptr;
    }

};

template <class T>
class WeakPtr {
    friend class SharedPtr<T>;
public:
    WeakPtr() : _ptr(), _counter() {};
    WeakPtr(const WeakPtr& o) : _ptr(o._ptr), _counter(o._counter) 
    {
        EnlargeCounter();
    }
    WeakPtr& operator=(const WeakPtr& o)
    {
        if (_counter != o._counter)
        {
            this->~WeakPtr();
        }

        _ptr = o._ptr;
        _counter = o._counter;
        EnlargeCounter();
        return *this;
    }
    
    WeakPtr(WeakPtr&& o) : _ptr(o._ptr), _counter(o._counter)
    {
        EnlargeCounter();
        o.Reset();
    }
    
    WeakPtr& operator=(WeakPtr&& o) 
    {
        if (_counter != o._counter)
        {
            this->~WeakPtr();
        }

        _ptr = o._ptr;
        _counter = o._counter;
        EnlargeCounter();
        o.Reset();
        return *this;
    }
    
    WeakPtr(const SharedPtr<T>& o) : _ptr(o._ptr), _counter(o._counter)
    {
        EnlargeCounter();
    }
    
    WeakPtr& operator=(const SharedPtr<T>& o)
    {
        if (_counter != o._counter)
        {
            this->~WeakPtr();
        }

        _counter = o._counter;
        _ptr = o._ptr;
        EnlargeCounter();
        return *this;
    }
    
    // Replaces pointer with nullptr
    void Reset()
    {
        this->~WeakPtr();

        _counter = nullptr;
        _ptr = nullptr;
    }
    
    bool Expired() const
    {
        return (_counter == nullptr) || (_counter->strong == 0);
    }
    
    SharedPtr<T> Lock()
    {
        return SharedPtr<T>(*this);
    }
        
    ~WeakPtr ()
    {
        if (_counter != nullptr)
        {
            _counter->weak--;
            if (_counter->weak == 0)
            {
                delete _ptr;

                if (_counter->strong == 0)
                {
                    delete _counter;
                }
            }
        }
    }

private:
    T* _ptr;
    RefCntBlock* _counter;

    void EnlargeCounter()
    {
        if (_counter != nullptr)
        {
            _counter->weak++;
        }
    }

    T* Get()
    {
        return _ptr();
    }
};

template <class T>
SharedPtr<T>::SharedPtr(const WeakPtr<T>& o) : _ptr(o._ptr), _counter(o._counter)
{
    EnlargeCounter();
}
