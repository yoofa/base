/*
 * HelloWorld.java
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

package io.github.yoofa;

import android.os.Build;

public class HelloWorld {

  public static void main(String[] args) {
    Logging.d("@@", "Hello world, " + Build.MANUFACTURER + " " + Build.MODEL + "!");
  }
}
