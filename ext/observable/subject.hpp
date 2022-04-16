#ifndef SUBJECT_H
#define SUBJECT_H

#include "observer.hpp"

template <class T>
class Subject
{
public:
  Subject() {}
  virtual ~Subject() {}
  void subscribe(Observer<T> &observer)
  {
    observers.push_back(&observer);
  }
  void notifyObservers()
  {
    typename std::vector<Observer<T> *>::iterator it;
    for (it = observers.begin(); it != observers.end(); it++)
      (*it)->update(static_cast<T *>(this));
  }

private:
  std::vector<Observer<T> *> observers;
};

#endif
