/*
 * one_time_event.h
 */
#ifndef BASE_ONE_TIME_EVENT_H_
#define BASE_ONE_TIME_EVENT_H_

namespace ave {
namespace base {

class OneTimeEvent {
 public:
  OneTimeEvent() = default;
  
  bool operator()() {
    if (happened_) return false;
    happened_ = true;
    return true;
  }

 private:
  bool happened_ = false;
};

}  // namespace base
}  // namespace ave

#endif  // BASE_ONE_TIME_EVENT_H_
