#ifndef OBSERVER_H
#define OBSERVER_H

template <class T>
class Observer
{
public:
  Observer() {}
  virtual ~Observer() {}
  virtual void update(T *subject) = 0;
};

#endif
