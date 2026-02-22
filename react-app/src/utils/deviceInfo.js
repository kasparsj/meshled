const toNumberOrNull = (value) => {
    const num = Number(value);
    return Number.isFinite(num) ? num : null;
};

const firstNumber = (...values) => {
    for (const value of values) {
        const num = toNumberOrNull(value);
        if (num !== null) return num;
    }
    return null;
};

export const normalizeDeviceInfo = (rawInfo) => {
    const info = rawInfo && typeof rawInfo === "object" ? rawInfo : {};
    const wifi = info.wifi && typeof info.wifi === "object" ? info.wifi : {};
    const leds = info.leds && typeof info.leds === "object" ? info.leds : {};
    const fs = info.fs && typeof info.fs === "object" ? info.fs : {};

    const ledsPower = firstNumber(leds.pwr);
    const ledsCount = firstNumber(leds.count);
    const freeHeap = firstNumber(info.freeheap);
    const freeMemory = firstNumber(info.freeMemory, freeHeap !== null ? freeHeap / 1024 : null);
    const storageUsed = firstNumber(info.storageUsed, fs.u);
    const storageTotal = firstNumber(info.storageTotal, fs.t);
    const fps = firstNumber(info.fps);

    return {
        ...info,
        wifi: {
            ...wifi,
            ssid: wifi.ssid ?? null,
        },
        leds: {
            ...leds,
            pwr: ledsPower,
            count: ledsCount,
        },
        ip: info.ip ?? null,
        activeLights: info.activeLights ?? ledsCount,
        freeMemory,
        storageUsed,
        storageTotal,
        fps,
    };
};
