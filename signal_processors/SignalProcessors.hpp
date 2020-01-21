// Hey Emacs, this is -*- coding: utf-8; mode: c++ -*-
#ifndef __SignalProcessors_hpp__
#define __SignalProcessors_hpp__

#include <limits>
#include <vector>
#include <cmath>
#include <memory>
#include <functional>

// TODO: * Convert all processors to MilliSecond (to do that convert scope
//         processing to MilliSecond from Second).

namespace rh {

namespace signal_processors {

class Second {};

class MilliSecond {
 private:
  using Self = MilliSecond;

 public:
  using Value = double;

  MilliSecond() =default;

  MilliSecond(const Self& other)
      : m_value{other.m_value}
  {}

  explicit MilliSecond(Value value)
      : m_value{value}
  {}

  Value value() const {
    return m_value;
  }

  Value& operator *() {
    return m_value;
  }

  const Value& operator *() const {
    return m_value;
  }

 private:
  Value m_value{};
};

template<typename V>
class Buffered {
 public:
  using Value = V;
  using Buffer = std::vector<Value>;
  using BufferSPtr = std::shared_ptr<Buffer>;

  Buffered(size_t samplesToBuffer)
      : m_samplesToBuffer{samplesToBuffer}
  {
    bufferReset();
  }

  BufferSPtr bufferCopySPtr() const {
    return std::make_shared<Buffer>(m_buffer);
  }

  size_t samplesToBuffer() const {
    return m_samplesToBuffer;
  }

 protected:
  void samplesToBuffer(size_t value) {
    m_samplesToBuffer = value;
    bufferReset();
  }

  Buffer& buffer() {
    return m_buffer;
  }

  void bufferReset() {
    m_buffer.clear();
    m_bufferTimePoint = 0;
  }

  void bufferTimePoint(double value) {
    m_bufferTimePoint = value;
  }

  double bufferTimePoint() const {
    return m_bufferTimePoint;
  }

 private:
  Buffer m_buffer;
  double m_bufferTimePoint;
  size_t m_samplesToBuffer;
};

class ForceUpdated {
 public:
  ForceUpdated(double forceUpdateTimeInterval)
      : m_forceUpdateTimeInterval(forceUpdateTimeInterval)
  {}

 protected:
  bool forceUpdate(double timePoint) {
    double timeDuration = std::abs(timePoint - m_lastUpdateTimePoint);
    if(timeDuration >= m_forceUpdateTimeInterval) {
      m_lastUpdateTimePoint = timePoint;
      return true;
    }
    else return false;
  }

 private:
  const double m_forceUpdateTimeInterval;
  double m_lastUpdateTimePoint{0};
};

template<typename V>
class ChangeTrackerUnitlessBase {
 public:
  using Value = V;

  Value lastValue() {
    return m_lastValue;
  }

 protected:
  ChangeTrackerUnitlessBase(Value lastValueInitial)
      : m_lastValue(lastValueInitial)
  {}

  void lastValue(Value value) {
    m_lastValue = value;
  }

  double lastValueTimePoint() {
    return m_lastValueTimePoint;
  }

  void lastValueTimePoint(double value) {
    m_lastValueTimePoint = value;
  }

 private:
  Value m_lastValue;
  double m_lastValueTimePoint;
};

template<typename V, typename TU>
class ChangeTrackerBase;

template<typename V>
class ChangeTrackerBase<V, void> : public ChangeTrackerUnitlessBase<V> {
 private:
  using Base = ChangeTrackerUnitlessBase<V>;

 public:
  using Value = typename Base::Value;

 protected:
  ChangeTrackerBase(Value initialValue)
      : Base(initialValue)
  {}
};

template<typename V>
class ChangeTrackerBase<V, Second> : public ChangeTrackerUnitlessBase<V> {
 private:
  using Base = ChangeTrackerUnitlessBase<V>;

 public:
  using Value = typename Base::Value;

  double lastValueTimePointSecond() {
    return Base::lastValueTimePoint;
  }

 protected:
  ChangeTrackerBase(Value initialValue)
      : Base(initialValue)
  {}
};

template<typename V>
class ChangeTrackerBase<V, MilliSecond> : public ChangeTrackerUnitlessBase<V> {
 private:
  using Base = ChangeTrackerUnitlessBase<V>;

 public:
  using Value = typename Base::Value;

  double lastValueTimePointMilliSecond() {
    return Base::lastValueTimePoint;
  }

 protected:
  ChangeTrackerBase(Value initialValue)
      : Base(initialValue)
  {}
};

template<typename V, typename TU>
class ChangeTracker : public ChangeTrackerBase<V, TU> {
 private:
  using Base = ChangeTrackerBase<V, TU>;

 public:
  using Value = typename Base::Value;

  using ResultCallback =
    std::function<void(Value lastValue,
                       double lastValueTimePoint)>;

 protected:
  ChangeTracker(Value initialValue)
      : Base(initialValue)
  {}

  void process(const std::function<Value(size_t)>& dataSampleGetter,
               const std::function<double(size_t)>& timePointGetter,
               size_t samplesToProcess,
               const ResultCallback& resultCallback)
  {
    for(size_t i = 0; i < samplesToProcess; ++i) {
      Value sample = dataSampleGetter(i);
      double timePoint = timePointGetter(i);
      if(Base::lastValue() != sample) {
        Base::lastValue(sample);
        Base::lastValueTimePoint(timePoint);
        Base::resultCallback(sample, timePoint);
      }
    }
  }
};

template<typename V>
using ChangeTrackerMilliSecond = ChangeTracker<V, MilliSecond>;

template<typename V, typename TU>
class ChangeTrackerForceUpdated
    : public ChangeTracker<V, TU>,
      public ForceUpdated
{
 private:
  using BaseChangeTracker = ChangeTracker<V, TU>;
  using BaseForceUpdated = ForceUpdated;

 public:
  using Value = typename BaseChangeTracker::Value;
  using ResultCallback = typename BaseChangeTracker::ResultCallback;

 protected:
  ChangeTrackerForceUpdated(
    Value initialValue,
    double forceUpdateTimeInterval
  )
      : BaseChangeTracker(initialValue),
        BaseForceUpdated(forceUpdateTimeInterval)
  {}

  void process(
    const std::function<Value(size_t)>& dataSampleGetter,
    const std::function<double(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback
  ) {
    for(size_t i = 0; i < samplesToProcess; ++i) {
      Value sample = dataSampleGetter(i);
      double timePoint = timePointGetter(i);
      if(BaseChangeTracker::lastValue() != sample ||
         BaseForceUpdated::forceUpdate(timePoint))
      {
        BaseChangeTracker::lastValue(sample);
        BaseChangeTracker::lastValueTimePoint(timePoint);
        resultCallback(sample, timePoint);
      }
    }
  }
};

template<typename V>
using ChangeTrackerForceUpdatedSecond =
  ChangeTrackerForceUpdated<V, Second>;

template<typename V>
using ChangeTrackerForceUpdatedMilliSecond =
  ChangeTrackerForceUpdated<V, MilliSecond>;

template<typename V>
class TimeWindowPeakToPeakTracker {
 public:
  using Value = V;

  using ResultCallback = std::function<
    void(
      Value lastPeakToPeakValue,
      Value lastMinValue,
      Value lastMaxValue,
      double lastMinValueTimePoint,
      double lastMaxValueTimePoint
    )
  >;

  Value lastPeakToPeakValue() const {
    return m_lastMaxValue - m_lastMinValue;
  }

  Value lastMinValue() const {
    return m_lastMinValue;
  }

  Value lastMaxValue() const {
    return m_lastMaxValue;
  }

 protected:
  TimeWindowPeakToPeakTracker(double timeWindowToTrack)
      : m_timeWindowToTrack(timeWindowToTrack)
  {
    resetValues();
  }

  void timeWindowToTrack(double value) const {
    m_timeWindowToTrack = value;
    resetValues();
  }

  double timeWindowToTrack() const {
    return m_timeWindowToTrack;
  }

  double lastMinValueTimePoint() const {
    return m_lastMinValueTimePoint;
  }

  double lastMaxValueTimePoint() const {
    return m_lastMaxValueTimePoint;
  }

  void process(
    const std::function<Value(size_t)>& dataSampleGetter,
    const std::function<double(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback
  ) {
    for(size_t i = 0; i < samplesToProcess; ++i) {
      if(
        updateLastValuesUsingTimeWindow(
          dataSampleGetter(i), timePointGetter(i))
      ) {
        resultCallback(
          lastPeakToPeakValue(), lastMinValue(), lastMaxValue(),
          lastMinValueTimePoint(), lastMaxValueTimePoint());
      }
    }
  }

 private:
  bool updateLastValuesUsingTimeWindow(Value value, double timePoint) {
    track(value, timePoint);
    if((timePoint - m_valuesUpdateTimePoint) >= m_timeWindowToTrack) {
      m_valuesUpdateTimePoint = timePoint;
      if(!updateLastValues()) {
        resetValues();
        track(value, timePoint);
        return false;
      }
      return true;
    }
    return false;
  }

  void track(Value value, double timePoint) {
    if(value > m_maxValue) {
      m_maxValue = value;
      m_maxValueTimePoint = timePoint;
    }
    if(value < m_minValue) {
      m_minValue = value;
      m_minValueTimePoint = timePoint;
    }
  }

  bool updateLastValues() {
    bool updated = false;
    if(m_lastMaxValue != m_maxValue) {
      m_lastMaxValue = m_maxValue;
      m_lastMaxValueTimePoint = m_maxValueTimePoint;
      updated = true;
    }
    if(m_lastMinValue != m_minValue) {
      m_lastMinValue = m_minValue;
      m_lastMinValueTimePoint = m_minValueTimePoint;
      updated = true;
    }
    return updated;
  }

  void resetValues() {
    m_maxValue = std::numeric_limits<Value>::lowest();
    m_maxValueTimePoint = std::numeric_limits<double>::quiet_NaN();
    m_minValue = std::numeric_limits<Value>::max();
    m_minValueTimePoint = std::numeric_limits<double>::quiet_NaN();
  }

  const double m_timeWindowToTrack;
  double m_valuesUpdateTimePoint{0};

  Value m_maxValue;
  double m_maxValueTimePoint;
  Value m_minValue;
  double m_minValueTimePoint;

  Value m_lastMaxValue{std::numeric_limits<Value>::lowest()};
  double m_lastMaxValueTimePoint{std::numeric_limits<double>::quiet_NaN()};
  Value m_lastMinValue{std::numeric_limits<Value>::max()};
  double m_lastMinValueTimePoint{std::numeric_limits<double>::quiet_NaN()};
};

template<typename V>
class TimeWindowPeakToPeakTrackerSecond
    : public TimeWindowPeakToPeakTracker<V>
{
 private:
  using Base = TimeWindowPeakToPeakTracker<V>;

 public:
  void timeWindowToTrackSecond(double value) const {
    Base::timeWindowToTrack(value);
  }

  double timeWindowToTrackSecond() const {
    return Base::timeWindowToTrack();
  }

  double lastMinValueTimePointSecond() const {
    return Base::lastMinValueTimePoint();
  }

  double lastMaxValueTimePointSecond() const {
    return Base::lastMaxValueTimePoint();
  }

 protected:
  TimeWindowPeakToPeakTrackerSecond(double timeWindowToTrack)
      : Base(timeWindowToTrack)
  {}
};

template<typename V>
class TimeAccumulateProcessorUnitlessBase {
 public:
  using Value = V;

  using ResultCallback =
    std::function<void(Value lastValue, double lastValueTimePoint)>;

  Value lastValue() const {
    return m_lastValue;
  }

 protected:
  TimeAccumulateProcessorUnitlessBase(
    double timeDurationToProcess,
    Value lastValueInitial
  )
      : m_timeDurationToProcess{timeDurationToProcess},
        m_lastValue{lastValueInitial}
  {}

  virtual void accumulate(Value value) =0;
  virtual Value lastValueCompute() =0;
  virtual void accumulatorReset() =0;

  void timeDurationToProcess(double value) {
    m_timeDurationToProcess = value;
    accumulatorReset();
  }

  double timeDurationToProcess() const {
    return m_timeDurationToProcess;
  }

  double lastValueTimePoint() const {
    return m_lastValueTimePoint;
  }

  void lastValueTimePoint(double value) {
    m_lastValueTimePoint = value;
  }

  bool updateLastValue(double timePoint) {
    bool valueUpdated = false;
    double timeDuration = std::abs(timePoint - m_lastValueTimePoint);
    if(timeDuration >= m_timeDurationToProcess) {
      m_lastValue = lastValueCompute();
      m_lastValueTimePoint = timePoint;
      accumulatorReset();
      valueUpdated = true;
    }
    return valueUpdated;
  }

 private:
  double m_timeDurationToProcess;
  Value m_lastValue;
  double m_lastValueTimePoint{0};
};

template<typename V, typename TU>
class TimeAccumulateProcessorBase;

template<typename V>
class TimeAccumulateProcessorBase<V, void>
    : public TimeAccumulateProcessorUnitlessBase<V>
{
 private:
  using Base = TimeAccumulateProcessorUnitlessBase<V>;

 public:
  using Value = typename Base::Value;

  double lastValueTimePoint() const {
    return Base::lastValueTimePoint();
  }

 protected:
  TimeAccumulateProcessorBase(
    double timeDurationToProcess,
    Value lastValueInitial
  )
      : Base(timeDurationToProcess, lastValueInitial)
  {}

  void lastValueTimePoint(double value) {
    Base::lastValueTimePoint(value);
  }
};

template<typename V>
class TimeAccumulateProcessorBase<V, Second>
    : public TimeAccumulateProcessorUnitlessBase<V>
{
 private:
  using Base = TimeAccumulateProcessorUnitlessBase<V>;

  double lastValueTimePointSecond() const {
    return Base::lastValueTimePoint();
  }

 public:
  using Value = typename Base::Value;

  double timeDurationToProcessSecond() const {
    return Base::timeDurationToProcess();
  }

 protected:
  TimeAccumulateProcessorBase(
    double timeDurationToProcessSecond,
    Value lastValueInitial
  )
      : Base(timeDurationToProcessSecond, lastValueInitial)
  {}

  void lastValueTimePointSecond(double value) {
    Base::lastValueTimePoint(value);
  }

  void timeDurationToProcessSecond(double value) {
    Base::timeDurationToProcess(value);
  }
};

template<typename V>
class TimeAccumulateProcessorBase<V, MilliSecond>
    : public TimeAccumulateProcessorUnitlessBase<V>
{
 private:
  using Base = TimeAccumulateProcessorUnitlessBase<V>;

 public:
  using Value = typename Base::Value;

  double lastValueTimePointMilliSecond() const {
    return Base::lastValueTimePoint();
  }

  double timeDurationToProcessMilliSecond() const {
    return Base::timeDurationToProcess();
  }

 protected:
  TimeAccumulateProcessorBase(
    double timeDurationToProcessMilliSecond,
    Value lastValueInitial
  )
      : Base(timeDurationToProcessMilliSecond, lastValueInitial)
  {}

  void lastValueTimePointMilliSecond(double value) {
    Base::lastValueTimePoint(value);
  }

  void timeDurationToProcessMilliSecond(double value) {
    Base::timeDurationToProcess(value);
  }
};

template<typename V, typename TU>
class TimeAccumulateProcessor : public TimeAccumulateProcessorBase<V, TU> {
 private:
  using Base = TimeAccumulateProcessorBase<V, TU>;

 public:
  using Value = V;
  using ResultCallback = typename Base::ResultCallback;

 protected:
  TimeAccumulateProcessor(double timeDurationToProcess, Value lastValueInitial)
      : Base(timeDurationToProcess, lastValueInitial)
  {}

  void process(
    const std::function<Value(size_t)>& dataSampleGetter,
    const std::function<double(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback
  ) {
    m_processingStopped = false;
    for(size_t i = 0; i < samplesToProcess; ++i) {
      this->accumulate(dataSampleGetter(i));
      if(Base::updateLastValue(timePointGetter(i))) {
        resultCallback(Base::lastValue(), Base::lastValueTimePoint());
        if(m_processingStopped) break;
      }
    }
  }

  void stopProcessing() {
    m_processingStopped = true;
  }

 private:
  bool m_processingStopped;
};

template<typename V, typename TU>
class TimeAverager : public TimeAccumulateProcessor<V, TU> {
 private:
  using Base = TimeAccumulateProcessor<V, TU>;

 public:
  using Value = typename Base::Value;

 protected:
  TimeAverager(double timeDurationToAverage, Value lastValueInitial)
      : Base(timeDurationToAverage, lastValueInitial)
  {
    accumulatorReset();
  }

  void accumulate(Value value) override {
    m_accumulatedValuesSum += value;
    m_accumulatedValuesCount += 1;
  }

  Value lastValueCompute() override {
    return m_accumulatedValuesSum / m_accumulatedValuesCount;
  }

  void accumulatorReset() override {
    m_accumulatedValuesSum = 0;
    m_accumulatedValuesCount = 0;
  }

 private:
  Value m_accumulatedValuesSum;
  Value m_accumulatedValuesCount;
};

template<typename V>
using TimeAveragerSecond = TimeAverager<V, Second>;

template<typename V>
using TimeAveragerMilliSecond = TimeAverager<V, MilliSecond>;

template<typename V, typename TU>
class TimeRmseProcessor : public TimeAccumulateProcessor<V, TU> {
 private:
  using Base = TimeAccumulateProcessor<V, TU>;

 public:
  using Value = typename Base::Value;

 protected:
  TimeRmseProcessor(double timeDurationToProcess, Value lastValueInitial)
      : Base(timeDurationToProcess, lastValueInitial)
  {
    accumulatorReset();
  }

  void accumulate(Value value) override {
    m_accumulatedValuesSum += value;
    m_accumulatedValues.push_back(value);
  }

  Value lastValueCompute() override {
    size_t valuesCount = m_accumulatedValues.size();
    Value valuesMean = m_accumulatedValuesSum / valuesCount;
    Value valuesSqSum = 0;
    for(Value value : m_accumulatedValues) {
      valuesSqSum += std::pow(value - valuesMean, 2);
    }
    return std::sqrt(valuesSqSum / valuesCount);
  }

  void accumulatorReset() override {
    m_accumulatedValuesSum = 0;
    m_accumulatedValues.clear();
  }

 private:
  using Values = std::vector<Value>;

  Value m_accumulatedValuesSum;
  Values m_accumulatedValues;
};

template<typename V>
using TimeRmseProcessorSecond = TimeRmseProcessor<V, Second>;

template<typename V, typename TU>
class TimeSdProcessor : public TimeAccumulateProcessor<V, TU> {
 private:
  using Base = TimeAccumulateProcessor<V, TU>;

 public:
  using Value = typename Base::Value;

  TimeSdProcessor(
    double timeDurationToProcess,
    Value lastValueInitial
  )
      : Base(timeDurationToProcess, lastValueInitial)
  {
    accumulatorReset();
  }

 protected:
  void accumulate(Value value) override {
    m_accumulatedValuesSum += value;
    m_accumulatedValues.push_back(value);
  }

  Value lastValueCompute() override {
    size_t valuesCount = m_accumulatedValues.size();
    Value valuesMean = m_accumulatedValuesSum / valuesCount;
    Value valuesAbsSum = 0;
    for(Value value : m_accumulatedValues) {
      valuesAbsSum += std::abs(value - valuesMean);
    }
    return valuesAbsSum / valuesCount;
  }

  void accumulatorReset() override {
    m_accumulatedValuesSum = 0;
    m_accumulatedValues.clear();
  }

 private:
  using Values = std::vector<Value>;

  Value m_accumulatedValuesSum;
  Values m_accumulatedValues;
};

template<typename V>
using TimeSecondSdProcessor = TimeSdProcessor<V, Second>;

template<typename V>
class ForwardProcessorBuffered : public Buffered<V> {
 private:
  using Base = Buffered<V>;

 public:
  using Value = typename Base::Value;
  using BufferSPtr = typename Base::BufferSPtr;

  using ResultCallback = std::function<
    void(BufferSPtr bufferCopySPtr, double bufferTimePoint)
  >;

  Value lastValue() const {
    return m_lastValue;
  }

  size_t samplesToBuffer() const {
    return Base::samplesToBuffer();
  }

  void reset() {
    Base::bufferReset();
  }

 protected:
  ForwardProcessorBuffered(
    size_t samplesToBuffer,
    Value lastValueInitial
  )
      : Base(samplesToBuffer),
        m_lastValue(lastValueInitial),
        m_lastValueTimePoint(0)
  {}

  void process(
    const std::function<Value(size_t)>& dataSampleGetter,
    const std::function<double(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback
  ) {
    for(size_t i = 0; i < samplesToProcess; ++i) {
      m_lastValue = dataSampleGetter(i);
      m_lastValueTimePoint = timePointGetter(i);
      Base::buffer().push_back(m_lastValue);
      if(Base::buffer().size() == 1) {
        Base::bufferTimePoint(m_lastValueTimePoint);
      }
      if(Base::buffer().size() == Base::samplesToBuffer()) {
        resultCallback(Base::bufferCopySPtr(), Base::bufferTimePoint());
        Base::buffer().clear();
      }
    }
  }

  void samplesToBuffer(size_t value) {
    Base::samplesToBuffer(value);
  }

  double lastValueTimePoint() const {
    return m_lastValueTimePoint;
  }

 private:
  Value m_lastValue;
  double m_lastValueTimePoint;
};

template<typename V>
class ForwardProcessorBufferedMilliSecond
    : public ForwardProcessorBuffered<V>
{
 private:
  using Base = ForwardProcessorBuffered<V>;

 public:
  using Value = typename Base::Value;

  double lastValueTimePointMilliSecond() const {
    return Base::lastValueTimePoint();
  }

  double bufferTimePointMilliSecond() const {
    return Base::bufferTimePoint();
  }

 protected:
  ForwardProcessorBufferedMilliSecond(
    size_t samplesToBuffer,
    Value lastValueInitial
  )
      : Base(samplesToBuffer, lastValueInitial)
  {}
};

template<typename V, typename TU>
class TimeAveragerBuffered
    : public TimeAverager<V, TU>,
      public Buffered<V>
{
 private:
  using BaseTimeAverager = TimeAverager<V, TU>;
  using BaseBuffered = Buffered<V>;

 public:
  using Value = typename BaseTimeAverager::Value;
  using BufferSPtr = typename BaseBuffered::BufferSPtr;

  using ResultCallback = std::function<
    void(BufferSPtr bufferCopySPtr, double bufferTimePoint)
  >;

  size_t samplesToBuffer() {
    return BaseBuffered::samplesToBuffer();
  }

  void samplesToBuffer(size_t value) {
    BaseBuffered::samplesToBuffer(value);
    BaseTimeAverager::accumulatorReset();
  }

  void reset() {
    BaseBuffered::bufferReset();
    BaseTimeAverager::accumulatorReset();
  }

 protected:
  TimeAveragerBuffered(
    double timeDurationToAverage,
    size_t samplesToBuffer,
    Value lastValueInitial
  )
      : BaseTimeAverager(timeDurationToAverage, lastValueInitial),
        BaseBuffered(samplesToBuffer)
  {}

  void process(
    const std::function<Value(size_t)>& dataSampleGetter,
    const std::function<double(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback
  ) {
    using Avr = BaseTimeAverager;
    using Buf = BaseBuffered;
    for(size_t i = 0; i < samplesToProcess; ++i) {
      this->accumulate(dataSampleGetter(i));
      double timePoint = timePointGetter(i);
      if(Avr::updateLastValue(timePoint)) {
        Buf::buffer().push_back(Avr::lastValue());
        if(Buf::buffer().size() == 1) {
          Buf::bufferTimePoint(Avr::lastValueTimePoint());
        }
        if(Buf::buffer().size() == Buf::samplesToBuffer()) {
          resultCallback(Buf::bufferCopySPtr(), Buf::bufferTimePoint());
          Buf::buffer().clear();
        }
      }
    }
  }
};

template<typename V>
class TimeAveragerBufferedMilliSecond
    : public TimeAveragerBuffered<V, MilliSecond>
{
 private:
  using Base = TimeAveragerBuffered<V, MilliSecond>;

 public:
  using Value = typename Base::Value;

  double bufferTimePointMilliSecond() const {
    return Base::bufferTimePoint();
  }

  double samplingIntervalMilliSecond() const {
    return Base::samplingInterval();
  }

 protected:
  TimeAveragerBufferedMilliSecond(
    double timeDurationToAverage,
    size_t samplesToBuffer,
    Value lastValueInitial
  )
      : Base(timeDurationToAverage, samplesToBuffer, lastValueInitial)
  {}

  void samplingIntervalMilliSecond(double value) {
    Base::samplingInterval(value);
  }
};

template<typename T>
class ValueDecimateFilter {
 public:
  using Value = T;

  Value lastValue() {
    return m_lastValue;
  }

 protected:
  void lastValue(const Value& value) {
    m_lastValue = value;
  }

 private:
  Value m_lastValue{0};
};

template<typename V>
class TimeWindowRangeTracker {
 public:
  using Value = V;

  struct Range {
    Value min;
    Value max;

    Range(Value min, Value max)
        : min(min), max(max)
    {}
  };

  void range(const Range& value) {
    m_range = value;
  }

  void range(Value min, Value max) {
    m_range.min = min;
    m_range.max = max;
  }

  const Range& range() {
    return m_range;
  }

  using CrossedRangeFunction = std::function<
    void(Value crossingValue, double crossingTimePoint)
  >;

  CrossedRangeFunction wentOutOfRange;
  CrossedRangeFunction wentIntoRange;

  bool inRange() {
    return m_inRange;
  }

  bool outOfRange() {
    return !inRange();
  }

 protected:
  void inRangeTimeWindow(double value) {
    m_inRangeTimeWindow = value;
  }

  double inRangeTimeWindow() {
    return m_inRangeTimeWindow;
  }

  void outOfRangeTimeWindow(double value) {
    m_outOfRangeTimeWindow = value;
  }

  double outOfRangeTimeWindow() {
    return m_outOfRangeTimeWindow;
  }

  double inRangeDuration() {
    return m_inRangeDuration;
  }

  double outOfRangeDuration() {
    return m_outOfRangeDuration;
  }

  TimeWindowRangeTracker(
    const Range& range,
    double inRangeTimeWindow,
    double outOfRangeTimeWindow
  )
      : m_range(range),
        m_outOfRangeTimeWindow(outOfRangeTimeWindow),
        m_inRangeTimeWindow(inRangeTimeWindow)
  {}

  TimeWindowRangeTracker(
    Value min,
    Value max,
    double inRangeTimeWindow,
    double outOfRangeTimeWindow
  )
      : m_range(min, max),
        m_outOfRangeTimeWindow(outOfRangeTimeWindow),
        m_inRangeTimeWindow(inRangeTimeWindow)
  {}

  bool checkInRange(Value value) {
    return m_range.min < value && value < m_range.max;
  }

  // NOTE: dataSampleGetter() and timePointMilliSecondGetter() may not
  //       be virtual, as in the derived class they should be templates
  //       (e.g. getting typename ConstDataBufferSPtr, typename SlowData
  //       as template parameters).
  //       Therefore those functions are passed to process() as
  //       parameters using lambdas in inherited class.
  //       Alternatively, Curiously Recurring Template Pattern (CRTP)
  //       can be used to call template function members of the inherited
  //       classes.
  //       (e.g. http://stackoverflow.com/questions/262254/crtp-to-avoid-dynamic-polymorphism)
  //       IMHO however, CRTP is more difficult to read in this case as it
  //       becomes less obvious what inherited functions are called by the
  //       parent class. We may still switch to CRTP in future for better
  //       run-time performance.
  void process(
    std::function<Value(size_t)> dataSampleGetter,
    std::function<double(size_t)> timePointGetter,
    size_t samplesToProcess
  ) {
    for(size_t i = 0; i < samplesToProcess; ++i) {
      Value sample = dataSampleGetter(i);
      double timePoint = timePointGetter(i);
      if(checkInRange(sample)) {
        outOfRangeTrackerStop();
        if(inRangeTrackerRunning()) inRangeTrackerUpdate(timePoint);
        else {
          if(!m_inRange) inRangeTrackerStart(sample, timePoint);
          else inRangeTrackerUpdate(timePoint);
        }
      }
      else {
        inRangeTrackerStop();
        if(outOfRangeTrackerRunning()) outOfRangeTrackerUpdate(timePoint);
        else {
          if(m_inRange) outOfRangeTrackerStart(sample, timePoint);
          else outOfRangeTrackerUpdate(timePoint);
        }
      }
    }
  }

  void outOfRangeTrackerStart(Value value, double timePoint) {
    m_wentOutOfRangeValue = value;
    m_wentOutOfRangeTimePoint = timePoint;
    m_outOfRangeDuration = 0;
    if(m_inRangeTimeWindow > 0) {
      m_outOfRangeTrackerRunning = true;
    }
    else {
      m_inRange = false;
      if(wentOutOfRange) {
        wentOutOfRange(m_wentOutOfRangeValue,
                       m_wentOutOfRangeTimePoint);
      }
    }
  }

  void outOfRangeTrackerStop() {
    m_outOfRangeTrackerRunning = false;
  }

  void outOfRangeTrackerUpdate(double timePoint) {
    m_outOfRangeDuration =
      timePoint - m_wentOutOfRangeTimePoint;
    if(m_outOfRangeDuration >
       m_outOfRangeTimeWindow &&
       m_outOfRangeTrackerRunning)
    {
      m_inRange = false;
      outOfRangeTrackerStop();
      if(wentOutOfRange) {
        wentOutOfRange(m_wentOutOfRangeValue,
                       m_wentOutOfRangeTimePoint);
      }
    }
  }

  bool outOfRangeTrackerRunning() {
    return m_outOfRangeTrackerRunning;
  }

  void inRangeTrackerStart(Value value, double timePointMilliSecond) {
    m_wentIntoRangeValue = value;
    m_wentIntoRangeTimePoint = timePointMilliSecond;
    m_inRangeDuration = 0;
    if(m_inRangeTimeWindow > 0) {
      m_inRangeTrackerRunning = true;
    }
    else {
      m_inRange = true;
      if(wentIntoRange) {
        wentIntoRange(m_wentIntoRangeValue,
                      m_wentIntoRangeTimePoint);
      }
    }
  }

  void inRangeTrackerStop() {
    m_inRangeTrackerRunning = false;
  }

  void inRangeTrackerUpdate(double timePointMilliSecond) {
    m_inRangeDuration =
      timePointMilliSecond - m_wentIntoRangeTimePoint;
    if(m_inRangeDuration >
       m_inRangeTimeWindow &&
       m_inRangeTrackerRunning)
    {
      m_inRange = true;
      inRangeTrackerStop();
      if(wentIntoRange) {
        wentIntoRange(m_wentIntoRangeValue,
                      m_wentIntoRangeTimePoint);
      }
    }
  }

  bool inRangeTrackerRunning() {
    return m_inRangeTrackerRunning;
  }

 private:
  Range m_range;
  bool m_inRange {false};

  Value m_wentOutOfRangeValue{0};
  bool m_outOfRangeTrackerRunning{false};
  double m_wentOutOfRangeTimePoint{0};
  double m_outOfRangeDuration{0};
  double m_outOfRangeTimeWindow;

  Value m_wentIntoRangeValue{0};
  bool m_inRangeTrackerRunning{false};
  double m_wentIntoRangeTimePoint{0};
  double m_inRangeDuration{0};
  double m_inRangeTimeWindow;
};

template<typename V>
class TimeWindowPredicateTracker {
 public:
  using Value = V;
  enum class ChangeDirection {unchanged, falseTrue, trueFalse};

  using ResultCallback = std::function<
    void(ChangeDirection changeDirection, MilliSecond changeTimePoint)
  >;

  bool lastPredicateValue() const {
    return m_lastPredicateValue;
  }

  ChangeDirection lastPredicateValueChangeDirection() const {
    return m_lastPredicateValueChangeTimePoint;
  }

  MilliSecond lastPredicateValueChangeTimePoint() const {
    return m_lastPredicateValueChangeTimePoint;
  }

  MilliSecond lastPredicateValueCheckTimePoint() const {
    return m_lastPredicateValueCheckTimePoint;
  }

  void falseTrueTimeWindow(MilliSecond value) {
    m_falseTrueTimeWindow = value;
    reset();
  }

  MilliSecond falseTrueTimeWindow() const {
    return m_falseTrueTimeWindow;
  }

  void trueFalseTimeWindow(MilliSecond value) {
    m_trueFalseTimeWindow = value;
    reset();
  }

  MilliSecond trueFalseTimeWindow() const {
    return m_trueFalseTimeWindow;
  }

 protected:
  TimeWindowPredicateTracker(
    MilliSecond falseTrueTimeWindow,
    MilliSecond trueFalseTimeWindow,
    bool lastPredicateValueInitial
  )
      : m_falseTrueTimeWindow(falseTrueTimeWindow),
        m_trueFalseTimeWindow(trueFalseTimeWindow),
        m_lastPredicateValueInitial(lastPredicateValueInitial)
  {
    reset();
  }

  void set(
    MilliSecond falseTrueTimeWindow,
    MilliSecond trueFalseTimeWindow,
    bool lastPredicateValueInitial
  ) {
    m_falseTrueTimeWindow = falseTrueTimeWindow;
    m_trueFalseTimeWindow = trueFalseTimeWindow;
    m_lastPredicateValueInitial = lastPredicateValueInitial;
  }

  void reset() {
    m_lastPredicateValue = m_lastPredicateValueInitial;
    m_lastPredicateValueChangeDirection = ChangeDirection::unchanged;
    m_lastPredicateValueChangeTimePoint =
      MilliSecond(-std::numeric_limits<MilliSecond::Value>::infinity());
    m_lastPredicateValueCheckTimePoint =
      MilliSecond(-std::numeric_limits<MilliSecond::Value>::infinity());
    m_falseTrueTrackerRunning = false;
    m_trueFalseTrackerRunning = false;
    m_falseTrueTimePoint =
      MilliSecond(-std::numeric_limits<MilliSecond::Value>::infinity());
    m_trueFalseTimePoint =
      MilliSecond(-std::numeric_limits<MilliSecond::Value>::infinity());
  }

  void process(
    const std::function<bool(size_t)>& predicate,
    const std::function<MilliSecond(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback
  ) {
    for(size_t i = 0; i < samplesToProcess; ++i) {
      bool predicateValue = predicate(i);
      auto timePoint = timePointGetter(i);
      m_lastPredicateValueCheckTimePoint = timePoint;
      if(m_lastPredicateValue) {
        // trueFalse change
        if(m_lastPredicateValue != predicateValue) {
          falseTrueTrackerStop();
          if(trueFalseTrackerRunning())
            trueFalseTrackerUpdate(timePoint, resultCallback);
          else trueFalseTrackerStart(timePoint, resultCallback);
        }
      }
      else {
        // falseTrue change
        if(m_lastPredicateValue != predicateValue) {
          trueFalseTrackerStop();
          if(falseTrueTrackerRunning())
            falseTrueTrackerUpdate(timePoint, resultCallback);
          else falseTrueTrackerStart(timePoint, resultCallback);
        }
      }
    }
  }

 private:
  void falseTrueTrackerStart(
    MilliSecond timePoint,
    const ResultCallback& resultCallback
  ) {
    m_falseTrueTimePoint = timePoint;
    if(*m_falseTrueTimeWindow > 0) {
      m_falseTrueTrackerRunning = true;
    }
    else {
      m_lastPredicateValue = true;
      m_lastPredicateValueChangeTimePoint = timePoint;
      if(resultCallback) {
        resultCallback(
          ChangeDirection::falseTrue,
          m_lastPredicateValueChangeTimePoint);
      }
    }
  }

  void falseTrueTrackerUpdate(
    MilliSecond timePoint,
    const ResultCallback& resultCallback
  ) {
    if(m_falseTrueTrackerRunning &&
       *timePoint - *m_falseTrueTimePoint > *m_falseTrueTimeWindow
    ) {
      falseTrueTrackerStop();
      m_lastPredicateValue = true;
      m_lastPredicateValueChangeTimePoint = timePoint;
      if(resultCallback) {
        resultCallback(
          ChangeDirection::falseTrue,
          m_lastPredicateValueChangeTimePoint);
      }
    }
  }

  bool falseTrueTrackerRunning() {
    return m_falseTrueTrackerRunning;
  }

  void falseTrueTrackerStop() {
    m_falseTrueTrackerRunning = false;
  }

  void trueFalseTrackerStart(
    MilliSecond timePoint,
    const ResultCallback& resultCallback
  ) {
    m_trueFalseTimePoint = timePoint;
    if(*m_trueFalseTimeWindow > 0) {
      m_trueFalseTrackerRunning = true;
    }
    else {
      m_lastPredicateValue = false;
      m_lastPredicateValueChangeTimePoint = timePoint;
      if(resultCallback) {
        resultCallback(
          ChangeDirection::trueFalse,
          m_lastPredicateValueChangeTimePoint);
      }
    }
  }

  void trueFalseTrackerUpdate(
    MilliSecond timePoint,
    const ResultCallback& resultCallback
  ) {
    if(m_trueFalseTrackerRunning &&
       *timePoint - *m_trueFalseTimePoint > *m_trueFalseTimeWindow
    ) {
      trueFalseTrackerStop();
      m_lastPredicateValue = false;
      m_lastPredicateValueChangeTimePoint = timePoint;
      if(resultCallback) {
        resultCallback(
          ChangeDirection::trueFalse,
          m_lastPredicateValueChangeTimePoint);
      }
    }
  }

  bool trueFalseTrackerRunning() {
    return m_trueFalseTrackerRunning;
  }

  void trueFalseTrackerStop() {
    m_trueFalseTrackerRunning = false;
  }

  MilliSecond m_falseTrueTimeWindow;
  MilliSecond m_trueFalseTimeWindow;
  bool m_lastPredicateValueInitial;

  bool m_lastPredicateValue;
  ChangeDirection m_lastPredicateValueChangeDirection;
  MilliSecond m_lastPredicateValueChangeTimePoint;
  MilliSecond m_lastPredicateValueCheckTimePoint;
  bool m_falseTrueTrackerRunning;
  bool m_trueFalseTrackerRunning;
  MilliSecond m_falseTrueTimePoint;
  MilliSecond m_trueFalseTimePoint;
};

template<typename V>
class TimeWindowGreaterThanThresholdTracker
    : public TimeWindowPredicateTracker<V>
{
 private:
  using Base = TimeWindowPredicateTracker<V>;

 public:
  using Value = typename Base::Value;
  using ResultCallback = typename Base::ResultCallback;

  void thresholdDelta(Value value) {
    m_thresholdDelta = value;
    Base::reset();
  }

  Value thresholdDelta() {
    return m_thresholdDelta;
  }

 protected:
  TimeWindowGreaterThanThresholdTracker(
    MilliSecond falseTrueTimeWindow,
    MilliSecond trueFalseTimeWindow,
    bool lastPredicateValueInitial,
    Value thresholdDelta
  )
      : Base(falseTrueTimeWindow,
             trueFalseTimeWindow,
             lastPredicateValueInitial),
        m_thresholdDelta(thresholdDelta)
  {}

  void set(
    MilliSecond falseTrueTimeWindow,
    MilliSecond trueFalseTimeWindow,
    bool lastPredicateValueInitial,
    Value thresholdDelta
  ) {
    Base::set(falseTrueTimeWindow,
              trueFalseTimeWindow,
              lastPredicateValueInitial);
    m_thresholdDelta = thresholdDelta;
  }

  void process(
    const std::function<Value(size_t)>& dataSampleGetter,
    const std::function<Value(size_t)>& thresholdSampleGetter,
    const std::function<MilliSecond(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback
  ) {
    for(size_t i = 0; i < samplesToProcess; ++i) {
      Base::process(
        [this, &dataSampleGetter, &thresholdSampleGetter](size_t index) {
          return dataSampleGetter(index) >
                 thresholdSampleGetter(index) + m_thresholdDelta;
        }, timePointGetter, samplesToProcess, resultCallback);
    }
  }

 private:
  Value m_thresholdDelta;
};

template<typename V>
class TimeWindowRangeTrackerMilliSecond : public TimeWindowRangeTracker<V> {
 private:
  using Base = TimeWindowRangeTracker<V>;

 public:
  void inRangeTimeWindowMilliSecond(double value) {
    Base::inRangeTimeWindow(value);
  }

  double inRangeTimeWindowMilliSecond() {
    return Base::inRangeTimeWindow();
  }

  void outOfRangeTimeWindowMilliSecond(double value) {
    Base::outOfRangeTimeWindow(value);
  }

  double outOfRangeTimeWindowMilliSecond() {
    return Base::outOfRangeTimeWindow();
  }

  double inRangeDurationMilliSecond() {
    return Base::inRangeDuration();
  }

  double outOfRangeDurationMilliSecond() {
    return Base::outOfRangeDuration();
  }

 protected:
  template<typename ...Ts>
  TimeWindowRangeTrackerMilliSecond(Ts&&... params)
      : Base(std::forward<Ts>(params)...)
  {}
};

template<typename V>
class TimeCompareValueTracker {
 public:
  using Value = V;

  using ResultCallback =
    std::function<
      // TODO: this would need some more work, but
      //       "MilliSecond::Value lastValueTimePointMilliSecond"
      //       must be converted to "MilliSecond lastValueTimePoint".
      void(Value lastValue, MilliSecond::Value lastValueTimePointMilliSecond)
    >;

  Value lastValue() const {
    return m_lastValue;
  }

  MilliSecond lastValueTimePoint() const {
    return m_lastValueTimePoint;
  }

  void reset() {
    m_lastValue = m_lastValueInitial;
    m_lastValueTimePoint = MilliSecond{0};
    track();
  }

 protected:
  using Compare = bool(*)(Value winningValue, Value newValue);

  TimeCompareValueTracker(
    MilliSecond timeDurationToProcess,
    Value lastValueInitial,
    Value winningValueInitial
  )
      : m_timeDurationToProcess{timeDurationToProcess},
        m_lastValueInitial{lastValueInitial},
        m_winningValueInitial{winningValueInitial}
  {
    reset();
  }

  void process(
    const std::function<Value(size_t)>& dataSampleGetter,
    const std::function<MilliSecond(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback,
    const Compare& compare
  ) {
    for(size_t i = 0; i < samplesToProcess; ++i) {
      auto dataSample = dataSampleGetter(i);
      auto timePoint = timePointGetter(i);

      if(std::isnan(*m_timePointWhenTrackingStarted)) {
        m_timePointWhenTrackingStarted = timePoint;
      }

      if(compare(m_currentWinningValue, dataSample)) {
        m_currentWinningValue = dataSample;
        m_currentWinningValueTimePoint = timePoint;
      }

      MilliSecond timeDuration{
        std::abs(*timePoint - *m_timePointWhenTrackingStarted)};
      if(*m_timeDurationToProcess < *timeDuration) {
        m_lastValue = m_currentWinningValue;
        m_lastValueTimePoint = m_currentWinningValueTimePoint;
        track();

        resultCallback(m_lastValue, *m_lastValueTimePoint);
      }
    }
  }

  void track() {
    m_currentWinningValue = m_winningValueInitial;
    m_currentWinningValueTimePoint =
      MilliSecond{std::numeric_limits<MilliSecond::Value>::quiet_NaN()};
    m_timePointWhenTrackingStarted =
      MilliSecond{std::numeric_limits<MilliSecond::Value>::quiet_NaN()};
  }

 private:
  const MilliSecond m_timeDurationToProcess;
  const Value m_lastValueInitial;
  const Value m_winningValueInitial;

  Value m_currentWinningValue;
  MilliSecond m_currentWinningValueTimePoint;
  MilliSecond m_timePointWhenTrackingStarted;

  Value m_lastValue;
  MilliSecond m_lastValueTimePoint;
};

template<typename V>
class TimeMaxValueTracker : public TimeCompareValueTracker<V> {
 private:
  using Base = TimeCompareValueTracker<V>;

 public:
  using Value = typename Base::Value;
  using ResultCallback = typename Base::ResultCallback;

 protected:
  TimeMaxValueTracker(
    MilliSecond timeDurationToProcess,
    Value lastValueInitial
  )
      : Base{
          timeDurationToProcess,
          lastValueInitial,
          std::numeric_limits<Value>::lowest()}
  {}

  void process(
    const std::function<Value(size_t)>& dataSampleGetter,
    const std::function<MilliSecond(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback
  ) {
    Base::process(
      dataSampleGetter, timePointGetter,
      samplesToProcess, resultCallback,
      [](Value winningValue, Value newValue) {
        return winningValue < newValue;
      }
    );
  }
};

template<typename V>
class TimeMinValueTracker : public TimeCompareValueTracker<V> {
 private:
  using Base = TimeCompareValueTracker<V>;

 public:
  using Value = typename Base::Value;
  using ResultCallback = typename Base::ResultCallback;

 protected:
  TimeMinValueTracker(
    MilliSecond timeDurationToProcess,
    Value lastValueInitial
  )
      : Base{
          timeDurationToProcess,
          lastValueInitial,
          std::numeric_limits<Value>::max()}
  {}

  void process(
    const std::function<Value(size_t)>& dataSampleGetter,
    const std::function<MilliSecond(size_t)>& timePointGetter,
    size_t samplesToProcess,
    const ResultCallback& resultCallback
  ) {
    Base::process(
      dataSampleGetter, timePointGetter,
      samplesToProcess, resultCallback,
      [](Value winningValue, Value newValue) {
        return winningValue > newValue;
      }
    );
  }
};

} // namespace signal_processors

} // namespace rh

#endif // __SignalProcessors_hpp__
