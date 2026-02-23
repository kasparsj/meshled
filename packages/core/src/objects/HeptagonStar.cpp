#include "HeptagonStar.h"

uint16_t* HeptagonStar::getMirroredPixels(uint16_t pixel, LPOwner* mirrorFlipEmitter, bool mirrorRotate) {
    uint8_t pathIndex = getStarSegmentIndex(pixel);
    float progress = getProgressOnStarSegment(pathIndex, pixel);
    uint8_t i = 1;
    mirrorPixels[0] = 0;
    if (mirrorFlipEmitter != NULL) {
        if (mirrorFlipEmitter->getType() == LPOwner::TYPE_INTERSECTION) {
            uint8_t emitterIndex = static_cast<Intersection*>(mirrorFlipEmitter)->id / 2;
            if (emitterIndex < 7) {
                uint8_t mirrorIndex = ((emitterIndex + (emitterIndex - pathIndex) + 11) % 7);
                mirrorPixels[i++] = getPixelOnStarSegment(mirrorIndex, 1.0 - progress);
                mirrorPixels[0] += 1;
            }
        }
    }
    if (mirrorRotate) {
        uint8_t mirrorIndex = (pathIndex + 4) % 7;
        mirrorPixels[i++] = getPixelOnStarSegment(mirrorIndex, progress);
        mirrorPixels[0] += 1;
    }
    return mirrorPixels;
}

uint8_t HeptagonStar::getStarSegmentIndex(uint16_t pixel) const {
    uint8_t stripIndex = 0;
    for (size_t i = 0; i < conn[0].size(); i++) {
        if (pixel >= conn[0][i]->to->topPixel && pixel <= conn[0][(i + 3) % 7]->from->topPixel) {
            stripIndex = static_cast<uint8_t>(i);
        }
    }
    return stripIndex;
}

float HeptagonStar::getProgressOnStarSegment(uint8_t pathIndex, uint16_t pixel) const {
    Intersection *from = conn[0][pathIndex]->to;
    uint8_t toIndex = (pathIndex + 3) % 7;
    Intersection *to = conn[0][toIndex]->from;
    return (float) (pixel - from->topPixel) / (to->topPixel - from->topPixel);
}

uint16_t HeptagonStar::getPixelOnStarSegment(uint8_t pathIndex, float perc) const {
    Intersection *from = conn[0][pathIndex]->to;
    uint8_t toIndex = (pathIndex + 3) % 7;
    Intersection *to = conn[0][toIndex]->from;
    return from->topPixel + round((to->topPixel - from->topPixel) * perc);
}
