import { useCallback, useState } from 'react';
import { useDevice } from '../contexts/DeviceContext.jsx';

const DEFAULT_EMITTER_SETTINGS = {
    autoEnabled: false,
    minSpeed: 0.5,
    maxSpeed: 10,
    minDuration: 1920,
    maxDuration: 23040,
};

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
        // ignore and fall through
    }

    return `${fallbackMessage} (${response.status})`;
};

const ensureOk = async (response, fallbackMessage) => {
    if (!response.ok) {
        throw new Error(await parseResponseError(response, fallbackMessage));
    }
};

const useEmitter = () => {
    const { deviceFetch } = useDevice();
    const [settings, setSettings] = useState(DEFAULT_EMITTER_SETTINGS);
    const [saving, setSaving] = useState(false);
    const [error, setError] = useState('');

    const postValue = useCallback(async (path, value, fallbackMessage) => {
        const formData = new FormData();
        formData.append('value', String(value));
        const response = await deviceFetch(path, {
            method: 'POST',
            body: formData,
        });
        await ensureOk(response, fallbackMessage);
    }, [deviceFetch]);

    const updateMinSpeed = useCallback(async (value) => {
        if (!Number.isFinite(value) || value <= 0 || value >= settings.maxSpeed) {
            throw new Error('Min speed must be above 0 and below max speed.');
        }

        const previous = settings.minSpeed;
        setSettings((prev) => ({ ...prev, minSpeed: value }));
        setSaving(true);
        setError('');

        try {
            await postValue('/update_emitter_min_speed', value, 'Failed to update min speed');
        } catch (err) {
            setSettings((prev) => ({ ...prev, minSpeed: previous }));
            setError(err.message || 'Failed to update min speed');
            throw err;
        } finally {
            setSaving(false);
        }
    }, [postValue, settings.maxSpeed, settings.minSpeed]);

    const updateMaxSpeed = useCallback(async (value) => {
        if (!Number.isFinite(value) || value <= settings.minSpeed) {
            throw new Error('Max speed must be above min speed.');
        }

        const previous = settings.maxSpeed;
        setSettings((prev) => ({ ...prev, maxSpeed: value }));
        setSaving(true);
        setError('');

        try {
            await postValue('/update_emitter_max_speed', value, 'Failed to update max speed');
        } catch (err) {
            setSettings((prev) => ({ ...prev, maxSpeed: previous }));
            setError(err.message || 'Failed to update max speed');
            throw err;
        } finally {
            setSaving(false);
        }
    }, [postValue, settings.maxSpeed, settings.minSpeed]);

    const updateMinDuration = useCallback(async (value) => {
        if (!Number.isFinite(value) || value <= 0 || value >= settings.maxDuration) {
            throw new Error('Min duration must be above 0 and below max duration.');
        }

        const previous = settings.minDuration;
        setSettings((prev) => ({ ...prev, minDuration: value }));
        setSaving(true);
        setError('');

        try {
            await postValue('/update_emitter_min_dur', value, 'Failed to update min duration');
        } catch (err) {
            setSettings((prev) => ({ ...prev, minDuration: previous }));
            setError(err.message || 'Failed to update min duration');
            throw err;
        } finally {
            setSaving(false);
        }
    }, [postValue, settings.maxDuration, settings.minDuration]);

    const updateMaxDuration = useCallback(async (value) => {
        if (!Number.isFinite(value) || value <= settings.minDuration) {
            throw new Error('Max duration must be above min duration.');
        }

        const previous = settings.maxDuration;
        setSettings((prev) => ({ ...prev, maxDuration: value }));
        setSaving(true);
        setError('');

        try {
            await postValue('/update_emitter_max_dur', value, 'Failed to update max duration');
        } catch (err) {
            setSettings((prev) => ({ ...prev, maxDuration: previous }));
            setError(err.message || 'Failed to update max duration');
            throw err;
        } finally {
            setSaving(false);
        }
    }, [postValue, settings.maxDuration, settings.minDuration]);

    const toggleAuto = useCallback(async () => {
        const previous = settings.autoEnabled;
        setSettings((prev) => ({ ...prev, autoEnabled: !prev.autoEnabled }));
        setSaving(true);
        setError('');

        try {
            const response = await deviceFetch('/toggle_auto', {
                method: 'POST',
            });
            await ensureOk(response, 'Failed to toggle auto emitter');
        } catch (err) {
            setSettings((prev) => ({ ...prev, autoEnabled: previous }));
            setError(err.message || 'Failed to toggle auto emitter');
            throw err;
        } finally {
            setSaving(false);
        }
    }, [deviceFetch, settings.autoEnabled]);

    return {
        settings,
        saving,
        error,
        updateMinSpeed,
        updateMaxSpeed,
        updateMinDuration,
        updateMaxDuration,
        toggleAuto,
    };
};

export default useEmitter;
