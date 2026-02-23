#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../src/Config.h"
#include "../src/EmitParams.h"
#include "../src/Globals.h"
#include "../src/Palette.h"
#include "../src/State.h"
#include "../src/objects/Cross.h"
#include "../src/objects/Line.h"
#include "../src/objects/Triangle.h"

namespace {

int fail(const std::string& message) {
    std::cerr << "FAIL: " << message << std::endl;
    return 1;
}

bool isNonBlack(const ColorRGB& color) {
    return color.R > 0 || color.G > 0 || color.B > 0;
}

}  // namespace

int main() {
    std::srand(2);
    gMillis = 0;

    // Off-by-one regression: model selector should allow LAST enum values.
    {
        Line line(LINE_PIXEL_COUNT);
        EmitParams* lineParams = line.getModelParams(L_BOUNCE);
        if (lineParams->model != L_BOUNCE) {
            delete lineParams;
            return fail("Line::getModelParams should preserve L_BOUNCE model");
        }
        delete lineParams;
    }
    {
        Cross cross(CROSS_PIXEL_COUNT);
        EmitParams* crossParams = cross.getModelParams(C_DIAGONAL);
        if (crossParams->model != C_DIAGONAL) {
            delete crossParams;
            return fail("Cross::getModelParams should preserve C_DIAGONAL model");
        }
        delete crossParams;
    }
    {
        Triangle triangle(TRIANGLE_PIXEL_COUNT);
        EmitParams* triangleParams = triangle.getModelParams(T_COUNTER_CLOCKWISE);
        if (triangleParams->model != T_COUNTER_CLOCKWISE) {
            delete triangleParams;
            return fail("Triangle::getModelParams should preserve T_COUNTER_CLOCKWISE model");
        }
        delete triangleParams;
    }

    // Palette wrap behavior should be deterministic in repeat mode.
    {
        std::vector<ColorRGB> colors = {ColorRGB(1, 2, 3), ColorRGB(10, 20, 30)};
        const ColorRGB& wrapped = Palette::wrapColors(2, 0, colors, WRAP_REPEAT);
        if (wrapped.R != 1 || wrapped.G != 2 || wrapped.B != 3) {
            return fail("Palette::wrapColors repeat mode returned unexpected color");
        }
    }

    // Lifecycle smoke: emit should render at least one pixel and stopAll should clear active lists.
    {
        Line line(LINE_PIXEL_COUNT);
        State state(line);
        state.lightLists[0]->visible = false;  // Ignore background for this check.

        EmitParams params(0, 1.0f, 0x00FF00);
        params.setLength(3);
        params.noteId = 42;

        const int8_t listIndex = state.emit(params);
        if (listIndex < 0) {
            return fail("State::emit failed unexpectedly for valid line model");
        }

        state.update();

        uint16_t litPixels = 0;
        for (uint16_t p = 0; p < line.pixelCount; p++) {
            if (isNonBlack(state.getPixel(p))) {
                litPixels++;
            }
        }
        if (litPixels == 0) {
            return fail("State::update produced no lit pixels after emit");
        }

        // Same note ID should reuse the existing list slot.
        EmitParams paramsSameNote(0, 1.0f, 0x0000FF);
        paramsSameNote.setLength(5);
        paramsSameNote.noteId = 42;
        const int8_t reusedIndex = state.emit(paramsSameNote);
        if (reusedIndex != listIndex) {
            return fail("State should reuse list index for the same noteId");
        }

        state.stopAll();
        for (int i = 0; i < 8; i++) {
            gMillis += 250;
            state.update();
        }
        if (state.totalLights != 0) {
            return fail("State::stopAll did not clear active emitted lights");
        }
    }

    return 0;
}
