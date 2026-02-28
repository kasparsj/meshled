import { useCallback, useEffect, useState } from 'react';
import { useDevice } from '../contexts/DeviceContext.jsx';

const isAbortError = (error) => error?.name === 'AbortError';

const parseResponseError = async (response, fallbackMessage) => {
    const contentType = response.headers.get('content-type') || '';
    try {
        if (contentType.includes('application/json')) {
            const payload = await response.json();
            if (typeof payload?.error === 'string' && payload.error.trim().length > 0) {
                return payload.error;
            }
            if (typeof payload?.message === 'string' && payload.message.trim().length > 0) {
                return payload.message;
            }
        }

        const text = await response.text();
        if (text.trim().length > 0) {
            return text;
        }
    } catch {
        // Ignore parse errors and use fallback.
    }

    return `${fallbackMessage} (${response.status})`;
};

const usePalettes = () => {
    const [palettes, setPalettes] = useState([]);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    const fetchPalettes = useCallback(async ({ verbose = true, paletteName = null, signal } = {}) => {
        if (!selectedDevice) {
            setPalettes([]);
            setLoading(false);
            setError('No device selected');
            return [];
        }

        setLoading(true);
        setError(null);

        try {
            let url = '/get_palettes';
            const params = new URLSearchParams();

            if (verbose) {
                params.append('v', 'true');
            }

            if (paletteName) {
                params.append('name', paletteName);
            }

            if (params.toString()) {
                url += `?${params.toString()}`;
            }

            const response = await deviceFetch(url, { signal });
            if (!response.ok) {
                throw new Error(await parseResponseError(response, 'Failed to fetch palettes'));
            }

            const data = await response.json();
            const parsed = Array.isArray(data) ? data : [];
            setPalettes(parsed);
            return parsed;
        } catch (err) {
            if (isAbortError(err)) {
                return [];
            }
            setError(err.message || 'Failed to fetch palettes');
            return null;
        } finally {
            setLoading(false);
        }
    }, [deviceFetch, selectedDevice]);

    const deletePalette = useCallback(async (paletteIndex) => {
        try {
            const formData = new FormData();
            formData.append('index', paletteIndex.toString());
            const response = await deviceFetch('/delete_palette', {
                method: 'POST',
                body: formData,
            });

            if (!response.ok) {
                throw new Error(await parseResponseError(response, 'Failed to delete palette'));
            }

            const result = await response.json().catch(() => ({ success: true }));
            await fetchPalettes();
            setError(null);
            return result;
        } catch (err) {
            setError(err.message || 'Failed to delete palette');
            return null;
        }
    }, [deviceFetch, fetchPalettes]);

    const syncPalettes = useCallback(async (palettesData, push = false, pull = false) => {
        try {
            const params = new URLSearchParams();
            if (push) params.append('push', 'true');
            if (pull) params.append('pull', 'true');

            const response = await deviceFetch(`/sync_palettes?${params.toString()}`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(palettesData),
            });

            if (!response.ok) {
                throw new Error(await parseResponseError(response, 'Failed to sync palettes'));
            }

            const result = await response.json();

            if (pull && Array.isArray(result) && result.length > 0) {
                setPalettes((prev) => {
                    const byName = new Map(prev.map((palette) => [palette.name, palette]));
                    for (const palette of result) {
                        if (palette?.name) {
                            byName.set(palette.name, palette);
                        }
                    }
                    return Array.from(byName.values());
                });
            }

            setError(null);
            return result;
        } catch (err) {
            setError(err.message || 'Failed to sync palettes');
            return null;
        }
    }, [deviceFetch]);

    const savePalette = useCallback(async (paletteData) => {
        try {
            const response = await deviceFetch('/save_palette', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(paletteData),
            });

            if (!response.ok) {
                throw new Error(await parseResponseError(response, 'Failed to save palette'));
            }

            const result = await response.json().catch(() => ({ success: true }));
            await fetchPalettes();
            setError(null);
            return result;
        } catch (err) {
            setError(err.message || 'Failed to save palette');
            return null;
        }
    }, [deviceFetch, fetchPalettes]);

    useEffect(() => {
        if (!selectedDevice) {
            setPalettes([]);
            setLoading(false);
            setError('No device selected');
            return;
        }

        const controller = new AbortController();
        fetchPalettes({ signal: controller.signal });

        return () => {
            controller.abort();
        };
    }, [selectedDevice, fetchPalettes]);

    return {
        palettes,
        loading,
        error,
        fetchPalettes,
        savePalette,
        deletePalette,
        syncPalettes,
    };
};

export default usePalettes;
