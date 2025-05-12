/*
 *  Copyright 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package io.github.yoofa;

/**
 * Class for representing size of an object. Very similar to android.util.Size but available on all
 * devices.
 */
public class Size {
    public int width;
    public int height;

    /**
     * Constructs a Size instance with the specified width and height.
     *
     * @param width the width dimension
     * @param height the height dimension
     */
    public Size(int width, int height) {
        this.width = width;
        this.height = height;
    }

    /**
     * Returns a string representation of the size in the format "widthxheight".
     *
     * @return a string combining the width and height separated by 'x'
     */
    @Override
    public String toString() {
        return width + "x" + height;
    }

    /**
     * Determines whether this Size is equal to another object.
     *
     * @param other the object to compare with this Size
     * @return true if the other object is a Size with the same width and height; false otherwise
     */
    @Override
    public boolean equals(Object other) {
        if (!(other instanceof Size)) {
            return false;
        }
        final Size otherSize = (Size) other;
        return width == otherSize.width && height == otherSize.height;
    }

    /**
     * Computes a hash code for this Size based on its width and height.
     *
     * @return a hash code that uniquely represents the width and height values
     */
    @Override
    public int hashCode() {
        // Use prime close to 2^16 to avoid collisions for normal values less than 2^16.
        return 1 + 65537 * width + height;
    }
}
