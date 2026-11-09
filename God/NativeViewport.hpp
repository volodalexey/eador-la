#pragma once

// Fit the fixed 1024x768 game canvas without changing its 4:3 aspect ratio.
struct NativeViewport {
    int x, y, w, h;

    static constexpr NativeViewport fit(int width, int height) {
        const int units = (width / 4 < height / 3) ? width / 4 : height / 3;
        return {(width - units * 4) / 2, (height - units * 3) / 2,
                units * 4, units * 3};
    }

    constexpr int mouseX(int physicalX) const {
        return physicalX < x || physicalX >= x + w || w == 0
            ? -1 : (physicalX - x) * 1024 / w;
    }

    constexpr int mouseY(int physicalY) const {
        return physicalY < y || physicalY >= y + h || h == 0
            ? -1 : (physicalY - y) * 768 / h;
    }
};
