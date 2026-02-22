import React from "react";

const ColorPreviewStripe = ({ palette, className = "w-full h-2 bg-zinc-800 rounded overflow-hidden" }) => {
    return (
        <div className={className}>
            <div className="flex h-full">
                {palette?.colors ? (() => {
                    const segmentation = palette.segmentation || 0;
                    
                    if (segmentation > 0) {
                        // Segmented palette behavior depends on wrap mode
                        const numSegments = Math.max(2, Math.round(segmentation));
                        const wrapMode = palette.wrapMode || 0;
                        const segments = [];
                        
                        if (wrapMode === -1) {
                            // Nowrap: first segment contains the whole palette, others are empty
                            for (let i = 0; i < numSegments; i++) {
                                if (i === 0) {
                                    // First segment: show the entire palette
                                    const segmentColors = [];
                                    for (let j = 0; j < palette.colors.length; j++) {
                                        segmentColors.push(
                                            <div
                                                key={`0-${j}`}
                                                style={{
                                                    backgroundColor: palette.colors[j],
                                                    width: `${100 / (numSegments * palette.colors.length)}%`
                                                }}
                                            />
                                        );
                                    }
                                    segments.push(...segmentColors);
                                } else {
                                    // Other segments: empty/transparent
                                    segments.push(
                                        <div
                                            key={i}
                                            style={{
                                                backgroundColor: 'transparent',
                                                width: `${100 / numSegments}%`,
                                                border: '1px solid rgba(255,255,255,0.1)'
                                            }}
                                        />
                                    );
                                }
                            }
                        } else if (wrapMode === 0) {
                            // Clamp to Edge: first segment has palette, others repeat the last color
                            for (let i = 0; i < numSegments; i++) {
                                if (i === 0) {
                                    // First segment: show the entire palette
                                    for (let j = 0; j < palette.colors.length; j++) {
                                        segments.push(
                                            <div
                                                key={`0-${j}`}
                                                style={{
                                                    backgroundColor: palette.colors[j],
                                                    width: `${100 / (numSegments * palette.colors.length)}%`
                                                }}
                                            />
                                        );
                                    }
                                } else {
                                    // Other segments: repeat the last color
                                    const lastColor = palette.colors[palette.colors.length - 1];
                                    segments.push(
                                        <div
                                            key={i}
                                            style={{
                                                backgroundColor: lastColor,
                                                width: `${100 / numSegments}%`
                                            }}
                                        />
                                    );
                                }
                            }
                        } else if (wrapMode === 1) {
                            // Repeat: each segment shows the complete palette
                            for (let i = 0; i < numSegments; i++) {
                                for (let j = 0; j < palette.colors.length; j++) {
                                    segments.push(
                                        <div
                                            key={`${i}-${j}`}
                                            style={{
                                                backgroundColor: palette.colors[j],
                                                width: `${100 / (palette.colors.length * numSegments)}%`
                                            }}
                                        />
                                    );
                                }
                            }
                        } else if (wrapMode === 2) {
                            // Repeat Mirror: alternate between forward and reverse palette
                            for (let i = 0; i < numSegments; i++) {
                                const isReversed = i % 2 === 1; // Odd segments are reversed
                                const colorsToShow = isReversed ? [...palette.colors].reverse() : palette.colors;
                                
                                for (let j = 0; j < colorsToShow.length; j++) {
                                    segments.push(
                                        <div
                                            key={`${i}-${j}`}
                                            style={{
                                                backgroundColor: colorsToShow[j],
                                                width: `${100 / (palette.colors.length * numSegments)}%`
                                            }}
                                        />
                                    );
                                }
                            }
                        }
                        
                        return segments;
                    } else {
                        // Smooth palette: show colors based on their positions
                        if (palette.positions && palette.positions.length === palette.colors.length && palette.positions.length > 1) {
                            return palette.colors.map((color, index) => {
                                const position = palette.positions[index];
                                const nextPosition = index < palette.colors.length - 1 ? palette.positions[index + 1] : 1;
                                const width = Math.max(0, (nextPosition - position) * 100);
                                
                                return (
                                    <div
                                        key={index}
                                        style={{
                                            backgroundColor: color,
                                            width: `${width}%`
                                        }}
                                    />
                                );
                            });
                        } else {
                            // Fallback: equal widths
                            return palette.colors.map((color, index) => (
                                <div
                                    key={index}
                                    style={{
                                        backgroundColor: color,
                                        width: `${100 / palette.colors.length}%`
                                    }}
                                />
                            ));
                        }
                    }
                })() : (
                    <div 
                        className="w-full"
                        style={{ backgroundColor: '#FF0000' }}
                    />
                )}
            </div>
        </div>
    );
};

export default ColorPreviewStripe;