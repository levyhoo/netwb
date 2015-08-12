#ifndef __SINGLETON_H__
#define __SINGLETON_H__
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

template<typename T>
class SingletonGuard : boost::mutex::scoped_lock, public boost::noncopyable
{
public:
    explicit SingletonGuard(T* inst, boost::mutex& mt):boost::mutex::scoped_lock(mt),m_guardPtr(inst)
    {         
    }
    T* operator->()
    {
        return m_guardPtr;
    }
private:
    T* m_guardPtr;
};

template<typename T>
class Singleton_wrapper : public T
{
public:
    static bool m_is_destroyed;
    ~Singleton_wrapper(){
        m_is_destroyed = true;
    }
};
template<typename T>
bool Singleton_wrapper< T >::m_is_destroyed = false;

template<typename T>
class Singleton : public boost::noncopyable
{
public:
    static SingletonGuard<T> getInstance(){
        return SingletonGuard<T>(&get_instance(), m_signalMutex);
    }
    static const T & get_const_instance(){
        return get_instance();
    }
private:
    static T & instance;
    static void use(T const &) {}
    static T & get_instance() {
        static Singleton_wrapper< T > t;
        BOOST_ASSERT(! Singleton_wrapper< T >::m_is_destroyed);
        use(instance);
        return static_cast<T &>(t);
    }
    static boost::mutex m_signalMutex; 
protected:
    boost::mutex::scoped_lock ScopedLock()
    {
        return boost::mutex::scoped_lock(m_signalMutex);
    }
};
template<typename T>
boost::mutex Singleton< T >::m_signalMutex; 
template<typename T>
T & Singleton< T >::instance = Singleton< T >::get_instance();
#endif //__SINGLETON_H__