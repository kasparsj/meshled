import { Timeline } from '@xzdarcy/react-timeline-editor';
import React from 'react';

const mockData = [{
    id: "0",
    actions: [
        {
            id: "action00",
            start: 0,
            end: 2,
            effectId: "effect0",
        },
    ],
},
    {
        id: "1",
        actions: [
            {
                id: "action10",
                start: 1.5,
                end: 5,
                effectId: "effect1",
            }
        ],
    }]

const mockEffect = {
    effect0: {
        id: "effect0",
        name: "效果0",
    },
    effect1: {
        id: "effect1",
        name: "效果1",
    },
};

const TimelineEditor = () => {
    return (
        <Timeline
            editorData={mockData}
            effects={mockEffect}
        />
    );
};

const TimelineTab = () => {
    return <TimelineEditor />
};

export default TimelineTab;