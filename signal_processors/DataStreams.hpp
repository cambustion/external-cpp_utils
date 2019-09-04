// Hey Emacs, this is -*- coding: utf-8; mode: c++ -*-
#ifndef __DataStreams_hpp__
#define __DataStreams_hpp__

#include <boost/signals2.hpp>

namespace rh {

namespace signal_processors {

class DataStream {
 public:
  struct DoubleTimed {
    DoubleTimed(double value, double timePointMilliSecond)
        : value(value), timePointMilliSecond(timePointMilliSecond)
    {}

    double value;
    double timePointMilliSecond;
  };

  using BufferAsDouble = std::vector<double>;
  using ConstBufferAsDouble = const BufferAsDouble;
  using ConstBufferAsDoubleSPtr = std::shared_ptr<ConstBufferAsDouble>;

  using EmitAsDoubleSignal =
    boost::signals2::signal<void(ConstBufferAsDoubleSPtr bufferAsDoubleSPtr,
                                 double bufferTimeMilliSecond)>;

  EmitAsDoubleSignal emitAsDouble;

  using ActiveChangedSignal = boost::signals2::signal<void(bool active)>;
  ActiveChangedSignal activeChanged;

  virtual size_t samplesPerTransaction() =0;
  virtual double samplingIntervalMilliSecond() =0;

  virtual void lastValueAsDouble(
    const std::function<void(DoubleTimed)>& callback) =0;
  virtual DoubleTimed lastValueAsDouble() = 0;

  using LastValueObserversCount = size_t;

  virtual LastValueObserversCount lastValueObserversCount() =0;
  virtual LastValueObserversCount lastValueObserverAdd() =0;
  virtual LastValueObserversCount lastValueObserverRemove() =0;

  virtual bool active() =0;
  virtual void active(const std::function<void(bool)>& callback) =0;

  virtual std::string dataDescription() const =0;
};

} // namespace signal_processors

} // namespace rh

#endif // __DataStreams_hpp__
