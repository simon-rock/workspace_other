// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include "DecayCounter.h"
#include <iostream>
using namespace std;

void DecayCounter::dump() const
{
  std::cout << "value : " << val << "delta : " << delta << "velocity : " << vel << std::endl;
}

void DecayCounter::generate_test_instances(list<DecayCounter*>& ls)
{
  utime_t fake_time;
  DecayCounter *counter = new DecayCounter(fake_time);
  counter->val = 3.0;
  counter->delta = 2.0;
  counter->vel = 1.0;
  ls.push_back(counter);
  counter = new DecayCounter(fake_time);
  ls.push_back(counter);
}

void DecayCounter::decay(utime_t now, const DecayRate &rate)
{
  if (now >= last_decay) {
    double el = (double)(now - last_decay);
    if (el >= 1.0) {
      // calculate new value
      double newval = (val+delta) * exp(el * rate.k);
      if (newval < .01)
	newval = 0.0;

      // calculate velocity approx
      vel += (newval - val) * el;
      vel *= exp(el * rate.k);

      val = newval;
      delta = 0;
      last_decay = now;
    }
  } else {
      last_decay = now;
  }
}
