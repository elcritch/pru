

#ifndef _SOFTWARE_SPI_HELPERS_H
#define _SOFTWARE_SPI_HELPERS_H

#include <stdint.h>
#include <iostream>

#ifndef DEBUG_STMT
#define debug(msg)
#endif

namespace SoftSPI {


template<uint32_t S0, uint32_t P0, uint32_t P1, uint32_t C0, uint32_t C1>
struct ClockTimings {
  // more precise than micro second delay,
  // 1/4 of SPI bus frequency , depends on MCU master clock,
  // const uint32_t sck_cycle;
  // propogation pre
  // const uint32_t prop_pre;
  // propogation post
  // const uint32_t prop_post;
  // capture pre (SCK edge -> capture) usually smaller delay
  // const uint32_t capt_pre;
  // capture post ( capture -> SCK edge)  usually bigger delay
  // const uint32_t capt_post;

  // ClockTimings()
    // : sck_cycle(0), prop_pre(0), prop_post(0), capt_pre(0), capt_post(0) {}

  // ClockTimings(uint32_t s0, uint32_t p0, uint32_t p1, uint32_t c0, uint32_t c1)
    // : sck_cycle(s0), prop_pre(p0), prop_post(p1), capt_pre(c0), capt_post(c1) {}

  static inline void delayCycles() { delayCycles(S0); }
  static inline void delayCyclesP0() { delayCycles(P0); }
  static inline void delayCyclesP1() { delayCycles(P1); }
  static inline void delayCyclesC0() { delayCycles(C0); }
  static inline void delayCyclesC1() { delayCycles(C1); }

  static void delayCycles(uint32_t cycles) {
    uint32_t i;
    for (i = 0; i < cycles; ++i) {
      NOOP;
    }
  }

  // static inline ClockTimings with_sck_cycle_and_pre_delays(uint32_t sck_cycle, uint32_t prop_pre, uint32_t capt_pre) {
    // return ClockTimings(sck_cycle,
                        // prop_pre,
                        // sck_cycle/2-prop_pre,
                        // prop_pre,
                        // sck_cycle/2-capt_pre);
  // }
};

// ========================================================================== //
// SPI Clock
// ========================================================================== //
template <Polarity CPOL>
struct SpiClock {

  const Pin sck;

  SpiClock(Pin _s) : sck(_s) {}

  inline void start();
  inline void stop();

  inline void tick() {
    start();
  }
  inline void tock() {
    stop();
  }

};

template <Polarity CPOL>
struct SpiClockToggler : SpiClock<CPOL> {

  SpiClockToggler(Pin _s) : SpiClock<CPOL>(_s) {}

  inline void tick() {
    digitalToggle(SpiClock<CPOL>::sck);
  }
  inline void tock() {
    digitalToggle(SpiClock<CPOL>::sck);
  }

};

//  clock inverted
template<>
inline void SpiClock<Inv>::start() {
  digitalWrite(sck, LOW);
}
template<>
inline void SpiClock<Inv>::stop() {
  digitalWrite(sck, HIGH);
}

//  clock standard
template<>
inline void SpiClock<Std>::start() {
  digitalWrite(sck, HIGH);
}
template<>
inline void SpiClock<Std>::stop() {
  digitalWrite(sck, LOW);
}


// ========================================================================== //
// SPI BitPack
// ========================================================================== //
template <BitOrder BITEND>
struct SpiPack {
  inline uint8_t mask(uint8_t byte, uint8_t idx, const uint32_t word_size);
  inline uint8_t pack(uint8_t bits[]);
};

template<>
inline uint8_t SpiPack<MsbFirst>::mask(uint8_t byte, uint8_t idx, const uint32_t word_size) {
  return (16*word_size >> idx) & byte;
}

template<>
inline uint8_t SpiPack<LsbFirst>::mask(uint8_t byte, uint8_t idx, const uint32_t word_size) {
  return (1 << idx) & byte;
}

template<>
inline uint8_t SpiPack<MsbFirst>::pack(uint8_t bits[])
{
  return (bits[0] << 7 | bits[1] << 6 | bits[2] << 5 | bits[3] << 4 | bits[4] << 3 | \
          bits[5] << 2 | bits[6] << 1 | bits[7]);
}

template<>
inline uint8_t SpiPack<LsbFirst>::pack(uint8_t bits[])
{
  return (bits[0] | bits[1] << 1 | bits[2] << 2 | bits[3] << 3 | bits[4] << 4 | \
          bits[5] << 5 | bits[6] << 6 | bits[7] << 7);
}

// ========================================================================== //
// SPI Xfer
// ========================================================================== //
template <PollEdge CPHA = Rising>
struct SpiXfer {

  template <class Clock, class Timings>
  inline uint8_t xfer_cycle( Clock clock, IOPins pins, bool bit);
};

template <>
template <class Clock, class Timings>
uint8_t SpiXfer<Falling>::xfer_cycle(Clock clock, IOPins pins, bool bit)
{
  bool read;

  clock.tick();

  Timings::delayCyclesP0();

  digitalWrite(pins.mosi, bit);

  // when PollEdge == Falling (CPOL=1) data will be captured at falling edge
  Timings::delayCyclesP1(); //  propagation

  clock.tock();

  Timings::delayCyclesC0(); // holding low, so there is enough time for data preparation and changing

  read = digitalRead(pins.miso); // reading at the middle of SCK pulse

  // wait until data is fetched by slave device,  while SCK low, checking DATAsheet for this interval
  Timings::delayCyclesC1();

  return read;
}

template <>
template <class Clock, class Timings>
uint8_t SpiXfer<Rising>::xfer_cycle( Clock clock, IOPins pins, bool bit)
{
  bool read;

  // changing MOSI big while SCK low, propogation
  digitalWrite(pins.mosi, bit);

  // there is a requirement of LOW and HIGH have identical interval!
  Timings::delayCyclesP1();

  clock.tick();
  // reading at the middle of SCK pulse
  Timings::delayCyclesC0();

  read = digitalRead(pins.miso); // reading at the middle of SCK pulse

  // wait until data is fetched by slave device,  while SCK high, checking DATAsheet for this interval
  Timings::delayCyclesC1();

  clock.tock();
  Timings::delayCyclesP0(); // holding low, so there is enough time for data preparation and changing

  return read;
}

}

#endif // header
