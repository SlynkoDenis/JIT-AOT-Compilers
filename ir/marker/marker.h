#ifndef JIT_AOT_COMPILERS_COURSE_MARKER_H_
#define JIT_AOT_COMPILERS_COURSE_MARKER_H_

#include <array>
#include <bitset>
#include <cstdint>
#include "helpers.h"
#include "macros.h"


namespace ir {
using Marker = uint32_t;

enum class MarkersConstants : uint32_t {
    BIT_SIZE = sizeof(Marker) * 8,
    BIT_LENGTH = 2,
    MAX_MARKERS = 1U << BIT_LENGTH,
    INDEX_MASK = MAX_MARKERS - 1U,
    MAX_VALUE = (1U << (BIT_SIZE - BIT_LENGTH)) - 1U,
    UNDEF_VALUE = 0,
};

class MarkerManager {
public:
    MarkerManager() = default;
    NO_COPY_SEMANTIC(MarkerManager);
    NO_MOVE_SEMANTIC(MarkerManager);
    virtual ~MarkerManager() = default;

    Marker GetNewMarker() {
        ASSERT(currentIndex < utils::to_underlying(MarkersConstants::MAX_VALUE));
        ++currentIndex;
        for (uint32_t i = 0; i < markersSlots.size(); ++i) {
            if (!markersSlots[i]) {
                Marker mark =
                    (currentIndex << utils::to_underlying(MarkersConstants::BIT_LENGTH)) | i;
                markersSlots[i] = true;
                return mark;
            }
        }
        UNREACHABLE("free marker slot not found");
        return 0;
    }
    void ReleaseMarker(Marker mark) {
        size_t index = mark & utils::to_underlying(MarkersConstants::INDEX_MASK);
        markersSlots[index] = false;
    }

private:
    size_t currentIndex = 0;
    std::bitset<utils::to_underlying(MarkersConstants::MAX_MARKERS)> markersSlots{false};
};

class Markable {
public:
    Markable() = default;
    NO_COPY_SEMANTIC(Markable);
    NO_MOVE_SEMANTIC(Markable);
    virtual ~Markable() noexcept = default;

    bool SetMarker(Marker mark) {
        auto value = mark >> utils::to_underlying(MarkersConstants::BIT_LENGTH);
        size_t index = mark & utils::to_underlying(MarkersConstants::INDEX_MASK);
        bool wasSet = markers.at(index) != value;
        markers.at(index) = value;
        return wasSet;
    }
    bool IsMarkerSet(Marker mark) const {
        auto value = mark >> utils::to_underlying(MarkersConstants::BIT_LENGTH);
        size_t index = mark & utils::to_underlying(MarkersConstants::INDEX_MASK);
        return markers.at(index) == value;
    }
    void ClearMarker(Marker mark) {
        size_t index = mark & utils::to_underlying(MarkersConstants::INDEX_MASK);
        markers.at(index) = utils::to_underlying(MarkersConstants::UNDEF_VALUE);
    }

private:
    std::array<Marker, utils::to_underlying(MarkersConstants::MAX_MARKERS)> markers;
};
}   // namespace ir

#endif  // JIT_AOT_COMPILERS_COURSE_MARKER_H_
